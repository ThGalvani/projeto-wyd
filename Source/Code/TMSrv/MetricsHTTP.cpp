#include "MetricsHTTP.h"
#include "ServerMetrics.h"
#include "ItemMonitor.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

namespace MetricsHTTP {

//==============================================================================
// HTTPServer Implementation
//==============================================================================

HTTPServer::HTTPServer(int port)
	: running(false)
	, port(port)
	, listen_socket(INVALID_SOCKET)
{
}

HTTPServer::~HTTPServer()
{
	Stop();
}

bool HTTPServer::Start()
{
	if (running.load())
		return false;

	// Inicia thread do servidor
	running = true;
	server_thread = std::thread(&HTTPServer::ServerLoop, this);

	return true;
}

void HTTPServer::Stop()
{
	if (!running.load())
		return;

	running = false;

	// Fecha socket
	if (listen_socket != INVALID_SOCKET)
	{
		closesocket(listen_socket);
		listen_socket = INVALID_SOCKET;
	}

	// Aguarda thread terminar
	if (server_thread.joinable())
		server_thread.join();
}

void HTTPServer::ServerLoop()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		running = false;
		return;
	}

	// Cria socket
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == INVALID_SOCKET)
	{
		WSACleanup();
		running = false;
		return;
	}

	// Permite reusar porta
	int opt = 1;
	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

	// Bind
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(listen_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		closesocket(listen_socket);
		WSACleanup();
		running = false;
		return;
	}

	// Listen
	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(listen_socket);
		WSACleanup();
		running = false;
		return;
	}

	// Aceita conexões
	while (running.load())
	{
		// Set timeout para accept
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(listen_socket, &readfds);

		timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int result = select(0, &readfds, NULL, NULL, &tv);
		if (result <= 0)
			continue;

		SOCKET client = accept(listen_socket, NULL, NULL);
		if (client == INVALID_SOCKET)
			continue;

		// Processa requisição
		HandleRequest(client);

		// Fecha conexão
		closesocket(client);
	}

	closesocket(listen_socket);
	WSACleanup();
}

void HTTPServer::HandleRequest(SOCKET client_socket)
{
	char buffer[4096];
	int bytesRead = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

	if (bytesRead <= 0)
		return;

	buffer[bytesRead] = '\0';

	// Parse primeira linha (GET /path HTTP/1.1)
	std::string request(buffer);
	size_t firstSpace = request.find(' ');
	size_t secondSpace = request.find(' ', firstSpace + 1);

	if (firstSpace == std::string::npos || secondSpace == std::string::npos)
	{
		SendHTTPResponse(client_socket, 400, "text/plain", "Bad Request");
		return;
	}

	std::string method = request.substr(0, firstSpace);
	std::string path = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);

	if (method != "GET")
	{
		SendHTTPResponse(client_socket, 405, "text/plain", "Method Not Allowed");
		return;
	}

	// Rotas
	if (path == "/api/metrics" || path == "/api/metrics/")
	{
		std::string json = GenerateMetricsJSON();
		SendHTTPResponse(client_socket, 200, "application/json", json);
	}
	else if (path == "/api/items" || path == "/api/items/")
	{
		std::string json = GenerateItemCountsJSON();
		SendHTTPResponse(client_socket, 200, "application/json", json);
	}
	else if (path == "/api/alerts" || path == "/api/alerts/")
	{
		std::string json = GenerateAlertsJSON();
		SendHTTPResponse(client_socket, 200, "application/json", json);
	}
	else
	{
		SendHTTPResponse(client_socket, 404, "text/plain", "Not Found");
	}
}

std::string HTTPServer::GenerateMetricsJSON()
{
	auto snapshot = ServerMetrics::g_Metrics.GetSnapshot();

	std::ostringstream json;
	json << std::fixed << std::setprecision(2);

	json << "{\n";
	json << "  \"timestamp\": \"" << snapshot.timestamp_str << "\",\n";
	json << "  \"server_health\": {\n";
	json << "    \"active_players\": " << snapshot.active_players << ",\n";
	json << "    \"active_mobs\": " << snapshot.active_mobs << ",\n";
	json << "    \"items_on_ground\": " << snapshot.items_on_ground << "\n";
	json << "  },\n";
	json << "  \"saves\": {\n";
	json << "    \"total\": " << snapshot.total_saves << ",\n";
	json << "    \"successful\": " << snapshot.successful_saves << ",\n";
	json << "    \"failed\": " << snapshot.failed_saves << ",\n";
	json << "    \"timeout\": " << snapshot.timeout_saves << ",\n";
	json << "    \"avg_time_ms\": " << snapshot.avg_save_time_ms << ",\n";
	json << "    \"success_rate\": " << snapshot.save_success_rate << "\n";
	json << "  },\n";
	json << "  \"trades\": {\n";
	json << "    \"total\": " << snapshot.total_trades << ",\n";
	json << "    \"successful\": " << snapshot.successful_trades << ",\n";
	json << "    \"failed\": " << snapshot.failed_trades << ",\n";
	json << "    \"rollbacks\": " << snapshot.rollback_trades << ",\n";
	json << "    \"success_rate\": " << snapshot.trade_success_rate << "\n";
	json << "  },\n";
	json << "  \"items\": {\n";
	json << "    \"total_drops\": " << snapshot.total_drops << ",\n";
	json << "    \"total_gets\": " << snapshot.total_gets << ",\n";
	json << "    \"total_splits\": " << snapshot.total_splits << ",\n";
	json << "    \"failed_drops\": " << snapshot.failed_drops << ",\n";
	json << "    \"failed_gets\": " << snapshot.failed_gets << "\n";
	json << "  }\n";
	json << "}\n";

	return json.str();
}

std::string HTTPServer::GenerateItemCountsJSON()
{
	auto counts = ItemMonitor::g_ItemMonitor.GetAllItemCounts();

	std::ostringstream json;
	json << "{\n";
	json << "  \"last_scan\": " << ItemMonitor::g_ItemMonitor.GetLastScanTime() << ",\n";
	json << "  \"items\": [\n";

	for (size_t i = 0; i < counts.size(); i++)
	{
		const auto& item = counts[i];

		json << "    {\n";
		json << "      \"id\": " << item.item_id << ",\n";
		json << "      \"name\": \"" << item.item_name << "\",\n";
		json << "      \"inventory\": " << item.count_inventory << ",\n";
		json << "      \"cargo\": " << item.count_cargo << ",\n";
		json << "      \"equipped\": " << item.count_equipped << ",\n";
		json << "      \"ground\": " << item.count_ground << ",\n";
		json << "      \"total\": " << item.total_count << ",\n";
		json << "      \"delta\": " << item.delta << ",\n";
		json << "      \"alert\": " << (item.alert_triggered ? "true" : "false") << "\n";
		json << "    }";

		if (i < counts.size() - 1)
			json << ",";
		json << "\n";
	}

	json << "  ]\n";
	json << "}\n";

	return json.str();
}

std::string HTTPServer::GenerateAlertsJSON()
{
	auto alerts = ItemMonitor::g_ItemMonitor.GetRecentAlerts(20);

	std::ostringstream json;
	json << "{\n";
	json << "  \"alerts\": [\n";

	for (size_t i = 0; i < alerts.size(); i++)
	{
		const auto& alert = alerts[i];

		json << "    {\n";
		json << "      \"timestamp\": \"" << alert.timestamp_str << "\",\n";
		json << "      \"item_id\": " << alert.item_id << ",\n";
		json << "      \"item_name\": \"" << alert.item_name << "\",\n";
		json << "      \"old_count\": " << alert.old_count << ",\n";
		json << "      \"new_count\": " << alert.new_count << ",\n";
		json << "      \"delta\": " << alert.delta << ",\n";
		json << "      \"type\": \"" << alert.alert_type << "\",\n";
		json << "      \"message\": \"" << alert.message << "\"\n";
		json << "    }";

		if (i < alerts.size() - 1)
			json << ",";
		json << "\n";
	}

	json << "  ]\n";
	json << "}\n";

	return json.str();
}

void HTTPServer::SendHTTPResponse(SOCKET sock, int code, const std::string& content_type,
	const std::string& body)
{
	std::ostringstream response;

	// Status line
	response << "HTTP/1.1 " << code << " ";
	switch (code)
	{
	case 200: response << "OK"; break;
	case 400: response << "Bad Request"; break;
	case 404: response << "Not Found"; break;
	case 405: response << "Method Not Allowed"; break;
	default: response << "Unknown"; break;
	}
	response << "\r\n";

	// Headers
	response << "Content-Type: " << content_type << "\r\n";
	response << "Content-Length: " << body.length() << "\r\n";
	response << "Access-Control-Allow-Origin: *\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";

	// Body
	response << body;

	std::string resp_str = response.str();
	send(sock, resp_str.c_str(), (int)resp_str.length(), 0);
}

//==============================================================================
// Global Instance
//==============================================================================

HTTPServer* g_MetricsHTTP = nullptr;

bool InitMetricsHTTP(int port)
{
	if (g_MetricsHTTP != nullptr)
		return false;

	g_MetricsHTTP = new HTTPServer(port);
	return g_MetricsHTTP->Start();
}

void ShutdownMetricsHTTP()
{
	if (g_MetricsHTTP != nullptr)
	{
		g_MetricsHTTP->Stop();
		delete g_MetricsHTTP;
		g_MetricsHTTP = nullptr;
	}
}

} // namespace MetricsHTTP

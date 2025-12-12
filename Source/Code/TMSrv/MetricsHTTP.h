#ifndef _METRICSHTTP_H_
#define _METRICSHTTP_H_

#include <thread>
#include <atomic>
#include <string>

//==============================================================================
// FASE 3 - Servidor HTTP Minimalista para Dashboard
//
// Servidor HTTP simples que expõe métricas em JSON para o dashboard web
// Escuta em porta configurável (padrão: 8080)
//==============================================================================

namespace MetricsHTTP {

class HTTPServer {
private:
	std::thread server_thread;
	std::atomic<bool> running;
	int port;
	SOCKET listen_socket;

	// Thread do servidor
	void ServerLoop();

	// Processa requisição HTTP
	void HandleRequest(SOCKET client_socket);

	// Gera resposta JSON com métricas
	std::string GenerateMetricsJSON();

	// Gera resposta JSON com contagem de itens
	std::string GenerateItemCountsJSON();

	// Gera resposta JSON com alertas
	std::string GenerateAlertsJSON();

	// Envia resposta HTTP
	void SendHTTPResponse(SOCKET sock, int code, const std::string& content_type,
		const std::string& body);

public:
	HTTPServer(int port = 8080);
	~HTTPServer();

	// Inicia servidor em thread separada
	bool Start();

	// Para servidor
	void Stop();

	// Verifica se está rodando
	bool IsRunning() const { return running.load(); }

	// Obtém porta
	int GetPort() const { return port; }
};

// Instância global
extern HTTPServer* g_MetricsHTTP;

// Funções auxiliares
bool InitMetricsHTTP(int port = 8080);
void ShutdownMetricsHTTP();

} // namespace MetricsHTTP

#endif

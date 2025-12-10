#ifndef _SERVERMETRICS_H_
#define _SERVERMETRICS_H_

#include <mutex>
#include <chrono>
#include <map>
#include <string>
#include <atomic>

//==============================================================================
// FASE 3 - Sistema de Métricas e Monitoramento do Servidor
//
// Este sistema coleta e armazena métricas em tempo real sobre:
// - Performance (SaveUser timing, Trade timing)
// - Operações críticas (Trades, Drops, Gets)
// - Itens raros no servidor
// - Detecção de possíveis dupes
//==============================================================================

namespace ServerMetrics {

// Estrutura para métricas de performance
struct PerformanceMetrics {
	// SaveUserSync metrics
	std::atomic<uint64_t> total_saves;
	std::atomic<uint64_t> successful_saves;
	std::atomic<uint64_t> failed_saves;
	std::atomic<uint64_t> timeout_saves;
	std::atomic<double> avg_save_time_ms;

	// Trade metrics
	std::atomic<uint64_t> total_trades;
	std::atomic<uint64_t> successful_trades;
	std::atomic<uint64_t> failed_trades;
	std::atomic<uint64_t> rollback_trades;

	// Item operations
	std::atomic<uint64_t> total_drops;
	std::atomic<uint64_t> total_gets;
	std::atomic<uint64_t> total_splits;
	std::atomic<uint64_t> failed_drops;
	std::atomic<uint64_t> failed_gets;

	// Server health
	std::atomic<int> active_players;
	std::atomic<int> active_mobs;
	std::atomic<int> items_on_ground;

	PerformanceMetrics();
	void Reset();
};

// Estrutura para snapshot de métricas (thread-safe read)
struct MetricsSnapshot {
	// SaveUser stats
	uint64_t total_saves;
	uint64_t successful_saves;
	uint64_t failed_saves;
	uint64_t timeout_saves;
	double avg_save_time_ms;
	double save_success_rate;

	// Trade stats
	uint64_t total_trades;
	uint64_t successful_trades;
	uint64_t failed_trades;
	uint64_t rollback_trades;
	double trade_success_rate;

	// Item operation stats
	uint64_t total_drops;
	uint64_t total_gets;
	uint64_t total_splits;
	uint64_t failed_drops;
	uint64_t failed_gets;

	// Server health
	int active_players;
	int active_mobs;
	int items_on_ground;

	// Timestamp
	time_t timestamp;
	char timestamp_str[64];
};

// Sistema global de métricas
class MetricsCollector {
private:
	PerformanceMetrics metrics;
	std::mutex snapshot_mutex;

	// Running average calculation
	double total_save_time_ms;
	uint64_t save_time_samples;

public:
	MetricsCollector();

	// SaveUser metrics
	void RecordSaveSuccess(double time_ms);
	void RecordSaveFailure();
	void RecordSaveTimeout();

	// Trade metrics
	void RecordTradeSuccess();
	void RecordTradeFailure();
	void RecordTradeRollback();

	// Item operation metrics
	void RecordDrop(bool success);
	void RecordGet(bool success);
	void RecordSplit();

	// Server health updates
	void UpdateServerHealth(int players, int mobs, int items);

	// Get snapshot (thread-safe)
	MetricsSnapshot GetSnapshot();

	// Reset all metrics
	void Reset();
};

// Global instance
extern MetricsCollector g_Metrics;

} // namespace ServerMetrics

#endif

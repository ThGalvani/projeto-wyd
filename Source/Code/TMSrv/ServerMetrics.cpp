#include "ServerMetrics.h"
#include <ctime>
#include <cstring>

namespace ServerMetrics {

//==============================================================================
// PerformanceMetrics Implementation
//==============================================================================

PerformanceMetrics::PerformanceMetrics()
	: total_saves(0)
	, successful_saves(0)
	, failed_saves(0)
	, timeout_saves(0)
	, avg_save_time_ms(0.0)
	, total_trades(0)
	, successful_trades(0)
	, failed_trades(0)
	, rollback_trades(0)
	, total_drops(0)
	, total_gets(0)
	, total_splits(0)
	, failed_drops(0)
	, failed_gets(0)
	, active_players(0)
	, active_mobs(0)
	, items_on_ground(0)
{
}

void PerformanceMetrics::Reset()
{
	total_saves = 0;
	successful_saves = 0;
	failed_saves = 0;
	timeout_saves = 0;
	avg_save_time_ms = 0.0;

	total_trades = 0;
	successful_trades = 0;
	failed_trades = 0;
	rollback_trades = 0;

	total_drops = 0;
	total_gets = 0;
	total_splits = 0;
	failed_drops = 0;
	failed_gets = 0;
}

//==============================================================================
// MetricsCollector Implementation
//==============================================================================

MetricsCollector::MetricsCollector()
	: total_save_time_ms(0.0)
	, save_time_samples(0)
{
}

void MetricsCollector::RecordSaveSuccess(double time_ms)
{
	metrics.total_saves++;
	metrics.successful_saves++;

	// Update running average
	std::lock_guard<std::mutex> lock(snapshot_mutex);
	total_save_time_ms += time_ms;
	save_time_samples++;
	metrics.avg_save_time_ms = total_save_time_ms / save_time_samples;
}

void MetricsCollector::RecordSaveFailure()
{
	metrics.total_saves++;
	metrics.failed_saves++;
}

void MetricsCollector::RecordSaveTimeout()
{
	metrics.total_saves++;
	metrics.timeout_saves++;
}

void MetricsCollector::RecordTradeSuccess()
{
	metrics.total_trades++;
	metrics.successful_trades++;
}

void MetricsCollector::RecordTradeFailure()
{
	metrics.total_trades++;
	metrics.failed_trades++;
}

void MetricsCollector::RecordTradeRollback()
{
	metrics.rollback_trades++;
}

void MetricsCollector::RecordDrop(bool success)
{
	metrics.total_drops++;
	if (!success)
		metrics.failed_drops++;
}

void MetricsCollector::RecordGet(bool success)
{
	metrics.total_gets++;
	if (!success)
		metrics.failed_gets++;
}

void MetricsCollector::RecordSplit()
{
	metrics.total_splits++;
}

void MetricsCollector::UpdateServerHealth(int players, int mobs, int items)
{
	metrics.active_players = players;
	metrics.active_mobs = mobs;
	metrics.items_on_ground = items;
}

MetricsSnapshot MetricsCollector::GetSnapshot()
{
	std::lock_guard<std::mutex> lock(snapshot_mutex);

	MetricsSnapshot snapshot;

	// SaveUser stats
	snapshot.total_saves = metrics.total_saves.load();
	snapshot.successful_saves = metrics.successful_saves.load();
	snapshot.failed_saves = metrics.failed_saves.load();
	snapshot.timeout_saves = metrics.timeout_saves.load();
	snapshot.avg_save_time_ms = metrics.avg_save_time_ms.load();

	if (snapshot.total_saves > 0)
		snapshot.save_success_rate = (double)snapshot.successful_saves / snapshot.total_saves * 100.0;
	else
		snapshot.save_success_rate = 100.0;

	// Trade stats
	snapshot.total_trades = metrics.total_trades.load();
	snapshot.successful_trades = metrics.successful_trades.load();
	snapshot.failed_trades = metrics.failed_trades.load();
	snapshot.rollback_trades = metrics.rollback_trades.load();

	if (snapshot.total_trades > 0)
		snapshot.trade_success_rate = (double)snapshot.successful_trades / snapshot.total_trades * 100.0;
	else
		snapshot.trade_success_rate = 100.0;

	// Item operation stats
	snapshot.total_drops = metrics.total_drops.load();
	snapshot.total_gets = metrics.total_gets.load();
	snapshot.total_splits = metrics.total_splits.load();
	snapshot.failed_drops = metrics.failed_drops.load();
	snapshot.failed_gets = metrics.failed_gets.load();

	// Server health
	snapshot.active_players = metrics.active_players.load();
	snapshot.active_mobs = metrics.active_mobs.load();
	snapshot.items_on_ground = metrics.items_on_ground.load();

	// Timestamp
	snapshot.timestamp = time(nullptr);
	struct tm timeinfo;
	localtime_s(&timeinfo, &snapshot.timestamp);
	strftime(snapshot.timestamp_str, sizeof(snapshot.timestamp_str), "%Y-%m-%d %H:%M:%S", &timeinfo);

	return snapshot;
}

void MetricsCollector::Reset()
{
	std::lock_guard<std::mutex> lock(snapshot_mutex);
	metrics.Reset();
	total_save_time_ms = 0.0;
	save_time_samples = 0;
}

// Global instance
MetricsCollector g_Metrics;

} // namespace ServerMetrics

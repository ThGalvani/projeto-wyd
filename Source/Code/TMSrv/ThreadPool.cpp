#include "ThreadPool.h"
#include <sstream>
#include <iomanip>

namespace Performance {

// Inst�ncia global
ThreadPoolManager g_ThreadPools;

// ============================================================================
// THREAD POOL - IMPLEMENTA��O
// ============================================================================

ThreadPool::ThreadPool(size_t num_threads)
    : stop_flag(false), active_tasks(0), total_tasks_executed(0), task_sequence(0) {

    // Cria worker threads
    for (size_t i = 0; i < num_threads; i++) {
        workers.emplace_back([this] { WorkerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop_flag.store(true);
    }

    condition.notify_all();

    // Aguarda todos os threads finalizarem
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::WorkerLoop() {
    while (true) {
        PrioritizedTask prioritized_task([]() {}, TaskPriority::NORMAL, 0);

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // Aguarda at� ter tarefa ou parar
            condition.wait(lock, [this] {
                return stop_flag.load() || !task_queue.empty();
            });

            // Se parou e n�o h� mais tarefas, finaliza
            if (stop_flag.load() && task_queue.empty()) {
                return;
            }

            // Pega tarefa da fila
            if (!task_queue.empty()) {
                prioritized_task = std::move(const_cast<PrioritizedTask&>(task_queue.top()));
                task_queue.pop();
            } else {
                continue;  // N�o h� tarefas, volta a aguardar
            }
        }

        // Executa tarefa
        active_tasks.fetch_add(1);

        try {
            prioritized_task.task();
        } catch (...) {
            // Ignora exce��es (pode adicionar logging aqui)
        }

        active_tasks.fetch_sub(1);
        total_tasks_executed.fetch_add(1);
    }
}

void ThreadPool::Submit(Task task, TaskPriority priority) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);

        if (stop_flag.load()) {
            return;  // N�o aceita mais tarefas
        }

        int seq = task_sequence.fetch_add(1);
        task_queue.emplace(std::move(task), priority, seq);
    }

    condition.notify_one();
}

void ThreadPool::WaitAll() {
    // Aguarda at� todas as tarefas finalizarem
    while (true) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (task_queue.empty() && active_tasks.load() == 0) {
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int ThreadPool::GetPendingTasks() const {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return task_queue.size();
}

void ThreadPool::Pause() {
    stop_flag.store(true);
}

void ThreadPool::Resume() {
    stop_flag.store(false);
    condition.notify_all();
}

// ============================================================================
// THREAD POOL MANAGER - IMPLEMENTA��O
// ============================================================================

ThreadPoolManager::ThreadPoolManager() {
    // Pool de I/O: 4 threads (DB, arquivos)
    io_pool = std::make_unique<ThreadPool>(4);

    // Pool de computa��o: metade dos cores dispon�veis
    size_t compute_threads = std::max(2u, std::thread::hardware_concurrency() / 2);
    compute_pool = std::make_unique<ThreadPool>(compute_threads);
}

ThreadPoolManager::~ThreadPoolManager() {
    // Unique_ptr limpa automaticamente
}

void ThreadPoolManager::SubmitDatabaseTask(Task task, TaskPriority priority) {
    io_pool->Submit(task, priority);
}

void ThreadPoolManager::SubmitFileTask(Task task, TaskPriority priority) {
    io_pool->Submit(task, priority);
}

void ThreadPoolManager::SubmitComputeTask(Task task, TaskPriority priority) {
    compute_pool->Submit(task, priority);
}

void ThreadPoolManager::WaitAllPools() {
    io_pool->WaitAll();
    compute_pool->WaitAll();
}

std::string ThreadPoolManager::GenerateReport() const {
    std::ostringstream report;

    report << "=== RELAT�RIO DE THREAD POOLS ===\n\n";

    // IO Pool
    report << "I/O POOL:\n";
    report << "  Threads: " << io_pool->GetThreadCount() << "\n";
    report << "  Tarefas Ativas: " << io_pool->GetActiveTasks() << "\n";
    report << "  Tarefas Pendentes: " << io_pool->GetPendingTasks() << "\n";
    report << "  Total Executado: " << io_pool->GetTotalTasksExecuted() << "\n\n";

    // Compute Pool
    report << "COMPUTE POOL:\n";
    report << "  Threads: " << compute_pool->GetThreadCount() << "\n";
    report << "  Tarefas Ativas: " << compute_pool->GetActiveTasks() << "\n";
    report << "  Tarefas Pendentes: " << compute_pool->GetPendingTasks() << "\n";
    report << "  Total Executado: " << compute_pool->GetTotalTasksExecuted() << "\n";

    return report.str();
}

} // namespace Performance

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>

// FASE 8: Thread Pool para Opera��es I/O
// Evita criar/destruir threads constantemente
// Reutiliza threads para tarefas ass�ncronas

namespace Performance {

// ============================================================================
// TIPOS
// ============================================================================

using Task = std::function<void()>;

// Prioridade de tarefa
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

// ============================================================================
// ESTRUTURA DE TAREFA
// ============================================================================

struct PrioritizedTask {
    Task task;
    TaskPriority priority;
    int sequence;  // Para ordenar tarefas da mesma prioridade (FIFO)

    PrioritizedTask(Task t, TaskPriority p, int seq)
        : task(std::move(t)), priority(p), sequence(seq) {}

    // Comparador para priority_queue (maior prioridade primeiro)
    bool operator<(const PrioritizedTask& other) const {
        if (priority != other.priority) {
            return priority < other.priority;  // Maior prioridade primeiro
        }
        return sequence > other.sequence;  // FIFO dentro da mesma prioridade
    }
};

// ============================================================================
// CLASSE THREAD POOL
// ============================================================================

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::priority_queue<PrioritizedTask> task_queue;

    mutable std::mutex queue_mutex;
    std::condition_variable condition;

    std::atomic<bool> stop_flag;
    std::atomic<int> active_tasks;
    std::atomic<int> total_tasks_executed;
    std::atomic<int> task_sequence;

    // Fun��o de worker thread
    void WorkerLoop();

public:
    // Construtor: cria pool com N threads
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());

    // Destrutor: aguarda todas as tarefas e finaliza threads
    ~ThreadPool();

    // Submete tarefa sem retorno
    void Submit(Task task, TaskPriority priority = TaskPriority::NORMAL);

    // Submete tarefa com retorno (usando std::future)
    template<typename F, typename... Args>
    auto SubmitWithResult(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    // Aguarda todas as tarefas finalizarem
    void WaitAll();

    // Estat�sticas
    size_t GetThreadCount() const { return workers.size(); }
    int GetActiveTasks() const { return active_tasks.load(); }
    int GetPendingTasks() const;
    int GetTotalTasksExecuted() const { return total_tasks_executed.load(); }

    // Pausa processamento de novas tarefas
    void Pause();

    // Resume processamento
    void Resume();

    // Verifica se est� parado
    bool IsStopped() const { return stop_flag.load(); }
};

// ============================================================================
// IMPLEMENTA��O TEMPLATE
// ============================================================================

template<typename F, typename... Args>
auto ThreadPool::SubmitWithResult(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {

    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
        std::lock_guard<std::mutex> lock(queue_mutex);

        if (stop_flag.load()) {
            throw std::runtime_error("ThreadPool is stopped");
        }

        int seq = task_sequence.fetch_add(1);
        task_queue.emplace(
            [task]() { (*task)(); },
            TaskPriority::NORMAL,
            seq
        );
    }

    condition.notify_one();
    return result;
}

// ============================================================================
// GERENCIADOR GLOBAL
// ============================================================================

class ThreadPoolManager {
private:
    std::unique_ptr<ThreadPool> io_pool;      // Pool para I/O (DB, arquivos)
    std::unique_ptr<ThreadPool> compute_pool; // Pool para computa��o pesada

public:
    ThreadPoolManager();
    ~ThreadPoolManager();

    // Acesso aos pools
    ThreadPool& GetIOPool() { return *io_pool; }
    ThreadPool& GetComputePool() { return *compute_pool; }

    // Helpers para tarefas comuns
    void SubmitDatabaseTask(Task task, TaskPriority priority = TaskPriority::NORMAL);
    void SubmitFileTask(Task task, TaskPriority priority = TaskPriority::NORMAL);
    void SubmitComputeTask(Task task, TaskPriority priority = TaskPriority::NORMAL);

    // Aguarda todos os pools
    void WaitAllPools();

    // Estat�sticas
    std::string GenerateReport() const;
};

// Inst�ncia global
extern ThreadPoolManager g_ThreadPools;

// ============================================================================
// HELPERS PARA INTEGRA��O
// ============================================================================

// Executa tarefa de DB de forma ass�ncrona
inline void AsyncDatabaseOperation(Task task, TaskPriority priority = TaskPriority::NORMAL) {
    g_ThreadPools.GetIOPool().Submit(task, priority);
}

// Executa tarefa de arquivo de forma ass�ncrona
inline void AsyncFileOperation(Task task, TaskPriority priority = TaskPriority::NORMAL) {
    g_ThreadPools.GetIOPool().Submit(task, priority);
}

// Executa computa��o pesada de forma ass�ncrona
inline void AsyncComputation(Task task, TaskPriority priority = TaskPriority::NORMAL) {
    g_ThreadPools.GetComputePool().Submit(task, priority);
}

} // namespace Performance

#endif // THREAD_POOL_H

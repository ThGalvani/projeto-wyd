#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <mutex>
#include <memory>

// FASE 8: QuadTree Spatial Indexing
// Otimiza��o de buscas espaciais (O(log n) em vez de O(n))

namespace Spatial {

// ============================================================================
// CONSTANTES
// ============================================================================

const int MAX_OBJECTS_PER_NODE = 8;   // M�ximo de objetos antes de subdividir
const int MAX_TREE_DEPTH = 8;         // Profundidade m�xima da �rvore

// ============================================================================
// ESTRUTURAS
// ============================================================================

// Ponto 2D
struct Point {
    int x;
    int y;

    Point() : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}
};

// Ret�ngulo (bounding box)
struct Rect {
    int x;      // Canto superior esquerdo
    int y;
    int width;
    int height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(int _x, int _y, int _w, int _h) : x(_x), y(_y), width(_w), height(_h) {}

    // Verifica se cont�m ponto
    bool Contains(const Point& p) const {
        return (p.x >= x && p.x < x + width &&
                p.y >= y && p.y < y + height);
    }

    // Verifica se intersecta com outro ret�ngulo
    bool Intersects(const Rect& other) const {
        return !(other.x >= x + width ||
                 other.x + other.width <= x ||
                 other.y >= y + height ||
                 other.y + other.height <= y);
    }

    // Centro do ret�ngulo
    Point Center() const {
        return Point(x + width / 2, y + height / 2);
    }
};

// Objeto espacial (player ou mob)
struct SpatialObject {
    int id;          // ID do player/mob
    Point position;
    void* data;      // Ponteiro para dados adicionais (CMob*, etc)

    SpatialObject() : id(0), data(nullptr) {}
    SpatialObject(int _id, int x, int y, void* _data = nullptr)
        : id(_id), position(x, y), data(_data) {}
};

// ============================================================================
// CLASSE QUADTREE NODE
// ============================================================================

class QuadTreeNode {
private:
    Rect boundary;                              // Limites deste n�
    std::vector<SpatialObject> objects;         // Objetos neste n�
    std::unique_ptr<QuadTreeNode> children[4];  // 4 filhos (NW, NE, SW, SE)
    int depth;                                  // Profundidade atual
    bool subdivided;                            // Se foi subdividido

    // Subdivide este n� em 4 filhos
    void Subdivide();

public:
    QuadTreeNode(const Rect& bounds, int current_depth = 0);
    ~QuadTreeNode();

    // Insere objeto
    bool Insert(const SpatialObject& obj);

    // Remove objeto por ID
    bool Remove(int id);

    // Busca objetos em �rea
    void Query(const Rect& range, std::vector<SpatialObject>& result) const;

    // Busca objetos em raio
    void QueryRadius(const Point& center, int radius, std::vector<SpatialObject>& result) const;

    // Busca objeto por ID
    SpatialObject* Find(int id);

    // Limpa todos os objetos
    void Clear();

    // Debug: conta total de objetos
    int CountObjects() const;
};

// ============================================================================
// CLASSE QUADTREE PRINCIPAL
// ============================================================================

class QuadTree {
private:
    std::unique_ptr<QuadTreeNode> root;
    mutable std::mutex tree_mutex;
    Rect world_bounds;

public:
    QuadTree(int world_width, int world_height);
    ~QuadTree();

    // Insere objeto (thread-safe)
    bool Insert(int id, int x, int y, void* data = nullptr);

    // Remove objeto por ID (thread-safe)
    bool Remove(int id);

    // Atualiza posi��o de objeto (thread-safe)
    bool Update(int id, int new_x, int new_y);

    // Busca objetos em �rea retangular (thread-safe)
    std::vector<SpatialObject> QueryArea(int x, int y, int width, int height) const;

    // Busca objetos em raio circular (thread-safe)
    std::vector<SpatialObject> QueryRadius(int center_x, int center_y, int radius) const;

    // Busca objetos pr�ximos (usa raio padr�o)
    std::vector<SpatialObject> QueryNearby(int x, int y, int default_radius = 50) const;

    // Busca objeto espec�fico por ID (thread-safe)
    SpatialObject* Find(int id);

    // Limpa todas as estruturas (thread-safe)
    void Clear();

    // Debug
    int GetObjectCount() const;
};

// ============================================================================
// GERENCIADOR GLOBAL
// ============================================================================

class SpatialManager {
private:
    std::unique_ptr<QuadTree> player_tree;  // �rvore para players
    std::unique_ptr<QuadTree> mob_tree;     // �rvore para mobs
    std::unique_ptr<QuadTree> item_tree;    // �rvore para itens no ch�o

public:
    SpatialManager(int world_width, int world_height);
    ~SpatialManager();

    // Players
    void InsertPlayer(int player_id, int x, int y, void* player_data = nullptr);
    void RemovePlayer(int player_id);
    void UpdatePlayer(int player_id, int x, int y);
    std::vector<SpatialObject> GetNearbyPlayers(int x, int y, int radius = 50);

    // Mobs
    void InsertMob(int mob_id, int x, int y, void* mob_data = nullptr);
    void RemoveMob(int mob_id);
    void UpdateMob(int mob_id, int x, int y);
    std::vector<SpatialObject> GetNearbyMobs(int x, int y, int radius = 50);

    // Items
    void InsertItem(int item_id, int x, int y, void* item_data = nullptr);
    void RemoveItem(int item_id);
    std::vector<SpatialObject> GetNearbyItems(int x, int y, int radius = 50);

    // Busca gen�rica
    std::vector<SpatialObject> GetAllNearby(int x, int y, int radius = 50);

    // Limpa tudo
    void ClearAll();

    // Stats
    int GetPlayerCount() const;
    int GetMobCount() const;
    int GetItemCount() const;
};

// Inst�ncia global
extern SpatialManager g_SpatialIndex;

} // namespace Spatial

#endif // QUADTREE_H

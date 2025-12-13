#include "QuadTree.h"
#include <cmath>
#include <algorithm>

namespace Spatial {

// Inst�ncia global
SpatialManager g_SpatialIndex(4096, 4096);  // Mundo 4096x4096

// ============================================================================
// QUADTREE NODE - IMPLEMENTA��O
// ============================================================================

QuadTreeNode::QuadTreeNode(const Rect& bounds, int current_depth)
    : boundary(bounds), depth(current_depth), subdivided(false) {
    objects.reserve(MAX_OBJECTS_PER_NODE);
}

QuadTreeNode::~QuadTreeNode() {
    // Unique_ptr limpa automaticamente
}

void QuadTreeNode::Subdivide() {
    if (subdivided) return;

    int half_width = boundary.width / 2;
    int half_height = boundary.height / 2;
    int x = boundary.x;
    int y = boundary.y;

    // NW (Northwest)
    children[0] = std::make_unique<QuadTreeNode>(
        Rect(x, y, half_width, half_height),
        depth + 1
    );

    // NE (Northeast)
    children[1] = std::make_unique<QuadTreeNode>(
        Rect(x + half_width, y, half_width, half_height),
        depth + 1
    );

    // SW (Southwest)
    children[2] = std::make_unique<QuadTreeNode>(
        Rect(x, y + half_height, half_width, half_height),
        depth + 1
    );

    // SE (Southeast)
    children[3] = std::make_unique<QuadTreeNode>(
        Rect(x + half_width, y + half_height, half_width, half_height),
        depth + 1
    );

    subdivided = true;

    // Redistribui objetos existentes
    std::vector<SpatialObject> temp_objects = objects;
    objects.clear();

    for (const auto& obj : temp_objects) {
        // Tenta inserir nos filhos
        bool inserted = false;
        for (int i = 0; i < 4; i++) {
            if (children[i]->Insert(obj)) {
                inserted = true;
                break;
            }
        }

        // Se n�o coube em nenhum filho, mant�m aqui
        if (!inserted) {
            objects.push_back(obj);
        }
    }
}

bool QuadTreeNode::Insert(const SpatialObject& obj) {
    // Verifica se objeto est� dentro dos limites
    if (!boundary.Contains(obj.position)) {
        return false;
    }

    // Se j� subdivido, tenta inserir nos filhos
    if (subdivided) {
        for (int i = 0; i < 4; i++) {
            if (children[i]->Insert(obj)) {
                return true;
            }
        }

        // N�o coube em nenhum filho, insere aqui
        objects.push_back(obj);
        return true;
    }

    // Se ainda h� espa�o, insere aqui
    if (objects.size() < MAX_OBJECTS_PER_NODE || depth >= MAX_TREE_DEPTH) {
        objects.push_back(obj);
        return true;
    }

    // Sen�o, subdivide e tenta novamente
    Subdivide();
    return Insert(obj);
}

bool QuadTreeNode::Remove(int id) {
    // Remove deste n�
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (it->id == id) {
            objects.erase(it);
            return true;
        }
    }

    // Tenta remover dos filhos
    if (subdivided) {
        for (int i = 0; i < 4; i++) {
            if (children[i]->Remove(id)) {
                return true;
            }
        }
    }

    return false;
}

void QuadTreeNode::Query(const Rect& range, std::vector<SpatialObject>& result) const {
    // Se n�o intersecta, ignora
    if (!boundary.Intersects(range)) {
        return;
    }

    // Adiciona objetos deste n� que est�o no range
    for (const auto& obj : objects) {
        if (range.Contains(obj.position)) {
            result.push_back(obj);
        }
    }

    // Busca nos filhos
    if (subdivided) {
        for (int i = 0; i < 4; i++) {
            children[i]->Query(range, result);
        }
    }
}

void QuadTreeNode::QueryRadius(const Point& center, int radius, std::vector<SpatialObject>& result) const {
    // Cria bounding box do raio
    Rect range(
        center.x - radius,
        center.y - radius,
        radius * 2,
        radius * 2
    );

    // Se n�o intersecta, ignora
    if (!boundary.Intersects(range)) {
        return;
    }

    // Adiciona objetos dentro do raio
    int radius_squared = radius * radius;
    for (const auto& obj : objects) {
        int dx = obj.position.x - center.x;
        int dy = obj.position.y - center.y;
        int dist_squared = dx * dx + dy * dy;

        if (dist_squared <= radius_squared) {
            result.push_back(obj);
        }
    }

    // Busca nos filhos
    if (subdivided) {
        for (int i = 0; i < 4; i++) {
            children[i]->QueryRadius(center, radius, result);
        }
    }
}

SpatialObject* QuadTreeNode::Find(int id) {
    // Busca neste n�
    for (auto& obj : objects) {
        if (obj.id == id) {
            return &obj;
        }
    }

    // Busca nos filhos
    if (subdivided) {
        for (int i = 0; i < 4; i++) {
            SpatialObject* found = children[i]->Find(id);
            if (found != nullptr) {
                return found;
            }
        }
    }

    return nullptr;
}

void QuadTreeNode::Clear() {
    objects.clear();

    if (subdivided) {
        for (int i = 0; i < 4; i++) {
            children[i]->Clear();
            children[i].reset();
        }
        subdivided = false;
    }
}

int QuadTreeNode::CountObjects() const {
    int count = objects.size();

    if (subdivided) {
        for (int i = 0; i < 4; i++) {
            count += children[i]->CountObjects();
        }
    }

    return count;
}

// ============================================================================
// QUADTREE PRINCIPAL - IMPLEMENTA��O
// ============================================================================

QuadTree::QuadTree(int world_width, int world_height)
    : world_bounds(0, 0, world_width, world_height) {
    root = std::make_unique<QuadTreeNode>(world_bounds);
}

QuadTree::~QuadTree() {
    // Unique_ptr limpa automaticamente
}

bool QuadTree::Insert(int id, int x, int y, void* data) {
    std::lock_guard<std::mutex> lock(tree_mutex);

    SpatialObject obj(id, x, y, data);
    return root->Insert(obj);
}

bool QuadTree::Remove(int id) {
    std::lock_guard<std::mutex> lock(tree_mutex);
    return root->Remove(id);
}

bool QuadTree::Update(int id, int new_x, int new_y) {
    std::lock_guard<std::mutex> lock(tree_mutex);

    // Remove da posi��o antiga
    SpatialObject* obj = root->Find(id);
    if (obj == nullptr) return false;

    void* data = obj->data;
    root->Remove(id);

    // Insere na nova posi��o
    SpatialObject new_obj(id, new_x, new_y, data);
    return root->Insert(new_obj);
}

std::vector<SpatialObject> QuadTree::QueryArea(int x, int y, int width, int height) const {
    std::lock_guard<std::mutex> lock(tree_mutex);

    std::vector<SpatialObject> result;
    Rect range(x, y, width, height);
    root->Query(range, result);

    return result;
}

std::vector<SpatialObject> QuadTree::QueryRadius(int center_x, int center_y, int radius) const {
    std::lock_guard<std::mutex> lock(tree_mutex);

    std::vector<SpatialObject> result;
    Point center(center_x, center_y);
    root->QueryRadius(center, radius, result);

    return result;
}

std::vector<SpatialObject> QuadTree::QueryNearby(int x, int y, int default_radius) const {
    return QueryRadius(x, y, default_radius);
}

SpatialObject* QuadTree::Find(int id) {
    std::lock_guard<std::mutex> lock(tree_mutex);
    return root->Find(id);
}

void QuadTree::Clear() {
    std::lock_guard<std::mutex> lock(tree_mutex);
    root->Clear();
}

int QuadTree::GetObjectCount() const {
    std::lock_guard<std::mutex> lock(tree_mutex);
    return root->CountObjects();
}

// ============================================================================
// SPATIAL MANAGER - IMPLEMENTA��O
// ============================================================================

SpatialManager::SpatialManager(int world_width, int world_height) {
    player_tree = std::make_unique<QuadTree>(world_width, world_height);
    mob_tree = std::make_unique<QuadTree>(world_width, world_height);
    item_tree = std::make_unique<QuadTree>(world_width, world_height);
}

SpatialManager::~SpatialManager() {
    // Unique_ptr limpa automaticamente
}

// Players
void SpatialManager::InsertPlayer(int player_id, int x, int y, void* player_data) {
    player_tree->Insert(player_id, x, y, player_data);
}

void SpatialManager::RemovePlayer(int player_id) {
    player_tree->Remove(player_id);
}

void SpatialManager::UpdatePlayer(int player_id, int x, int y) {
    player_tree->Update(player_id, x, y);
}

std::vector<SpatialObject> SpatialManager::GetNearbyPlayers(int x, int y, int radius) {
    return player_tree->QueryRadius(x, y, radius);
}

// Mobs
void SpatialManager::InsertMob(int mob_id, int x, int y, void* mob_data) {
    mob_tree->Insert(mob_id, x, y, mob_data);
}

void SpatialManager::RemoveMob(int mob_id) {
    mob_tree->Remove(mob_id);
}

void SpatialManager::UpdateMob(int mob_id, int x, int y) {
    mob_tree->Update(mob_id, x, y);
}

std::vector<SpatialObject> SpatialManager::GetNearbyMobs(int x, int y, int radius) {
    return mob_tree->QueryRadius(x, y, radius);
}

// Items
void SpatialManager::InsertItem(int item_id, int x, int y, void* item_data) {
    item_tree->Insert(item_id, x, y, item_data);
}

void SpatialManager::RemoveItem(int item_id) {
    item_tree->Remove(item_id);
}

std::vector<SpatialObject> SpatialManager::GetNearbyItems(int x, int y, int radius) {
    return item_tree->QueryRadius(x, y, radius);
}

// Busca gen�rica
std::vector<SpatialObject> SpatialManager::GetAllNearby(int x, int y, int radius) {
    std::vector<SpatialObject> result;

    auto players = GetNearbyPlayers(x, y, radius);
    auto mobs = GetNearbyMobs(x, y, radius);
    auto items = GetNearbyItems(x, y, radius);

    result.insert(result.end(), players.begin(), players.end());
    result.insert(result.end(), mobs.begin(), mobs.end());
    result.insert(result.end(), items.begin(), items.end());

    return result;
}

void SpatialManager::ClearAll() {
    player_tree->Clear();
    mob_tree->Clear();
    item_tree->Clear();
}

int SpatialManager::GetPlayerCount() const {
    return player_tree->GetObjectCount();
}

int SpatialManager::GetMobCount() const {
    return mob_tree->GetObjectCount();
}

int SpatialManager::GetItemCount() const {
    return item_tree->GetObjectCount();
}

} // namespace Spatial

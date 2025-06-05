import heapq
from typing import Dict, List, Set, Tuple

class Graph:
    def __init__(self):
        self.vertices: Dict[str, Dict[str, int]] = {}
        
    def add_edge(self, from_vertex: str, to_vertex: str, weight: int):
        # Thêm cạnh hai chiều
        if from_vertex not in self.vertices:
            self.vertices[from_vertex] = {}
        if to_vertex not in self.vertices:
            self.vertices[to_vertex] = {}
            
        self.vertices[from_vertex][to_vertex] = weight
        self.vertices[to_vertex][from_vertex] = weight

def dijkstra(graph: Graph, start: str, end: str) -> Tuple[List[str], int]:
    # Khởi tạo khoảng cách và đường đi
    distances = {vertex: float('infinity') for vertex in graph.vertices}
    distances[start] = 0
    previous = {vertex: None for vertex in graph.vertices}
    
    # Hàng đợi ưu tiên
    priority_queue = [(0, start)]
    
    # Tập các đỉnh đã xét
    visited: Set[str] = set()
    
    while priority_queue:
        current_distance, current_vertex = heapq.heappop(priority_queue)
        
        # Nếu đã đến đích
        if current_vertex == end:
            break
            
        # Nếu đỉnh đã được xét hoặc khoảng cách hiện tại lớn hơn khoảng cách đã lưu
        if current_vertex in visited or current_distance > distances[current_vertex]:
            continue
            
        visited.add(current_vertex)
        
        # Xét các đỉnh kề
        for neighbor, weight in graph.vertices[current_vertex].items():
            if neighbor in visited:
                continue
                
            distance = current_distance + weight
            
            # Nếu tìm thấy đường đi ngắn hơn
            if distance < distances[neighbor]:
                distances[neighbor] = distance
                previous[neighbor] = current_vertex
                heapq.heappush(priority_queue, (distance, neighbor))
    
    # Tạo đường đi từ đích về xuất phát
    path = []
    current = end
    while current is not None:
        path.append(current)
        current = previous[current]
    path.reverse()
    
    return path, distances[end]

def create_hust_graph() -> Graph:
    graph = Graph()
    
    # Thêm các cạnh cho các tòa nhà
    # Các tòa C
    for i in range(1, 10):
        graph.add_edge(f"Toa C{i}", f"Toa C{i+1}", 50)
    
    # Các tòa D
    for i in range(3, 9):
        graph.add_edge(f"Toa D{i}", f"Toa D{i+1}", 50)
    
    # Kết nối giữa các tòa C và D
    for i in range(1, 8):
        graph.add_edge(f"Toa C{i}", f"Toa D{i+2}", 100)
    
    # Kết nối với Thư viện
    graph.add_edge("Toa C8", "Thu Vien", 150)
    graph.add_edge("Toa C9", "Thu Vien", 150)
    graph.add_edge("Toa C10", "Thu Vien", 150)
    
    # Kết nối với Nhà TC
    graph.add_edge("Toa D3", "Nha TC", 150)
    graph.add_edge("Toa D4", "Nha TC", 150)
    graph.add_edge("Toa D5", "Nha TC", 150)
    
    return graph

def find_shortest_path(source: str, destination: str) -> Tuple[List[str], int]:
    graph = create_hust_graph()
    return dijkstra(graph, source, destination)

if __name__ == "__main__":
    # Test với một số trường hợp
    test_cases = [
        ("Toa C1", "Toa C5"),
        ("Toa C1", "Thu Vien"),
        ("Toa D3", "Nha TC"),
        ("Toa C1", "Toa D9")
    ]
    
    for source, dest in test_cases:
        path, distance = find_shortest_path(source, dest)
        print(f"\nĐường đi từ {source} đến {dest}:")
        print(f"Đường đi: {' -> '.join(path)}")
        print(f"Khoảng cách: {distance} mét") 
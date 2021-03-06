#include "route_planner.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    // Find the closest nodes to the starting and ending coordinates
    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);
}


float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
    return node->distance(*end_node);
}


void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {
    // Populate current_node.neighbors vector with all the neighbors
    current_node->FindNeighbors();

    // Go through each current_node neighbors, set the parent, the h_value, the g_value
    // Add each neighbor to open_list and set the node's visited attribute to true.
    for(auto neighbor : current_node->neighbors){
        neighbor->parent = current_node;
        neighbor->h_value = CalculateHValue(neighbor);
        neighbor->g_value = current_node->g_value + current_node->distance(*neighbor);
        open_list.emplace_back(neighbor);
        neighbor->visited = true;
    }
}


RouteModel::Node *RoutePlanner::NextNode() {
    // Sort the open_list according to the sum of the h value and g value
    std::sort(open_list.begin(), open_list.end(), [](const auto &_1st , const auto &_2nd)
    {
        return (_1st->g_value + _1st->h_value) < (_2nd->g_value + _2nd->h_value);
    });
    // Create a pointer to the node in the list with the lowest sum and remove that node from the open_list
    RouteModel::Node *lowest_node = open_list.front();
    open_list.erase(open_list.begin())
    return lowest_node;
}


// This method takes the current (final) node as an argument and iteratively follow the chain of parents of nodes until the starting node is found
// The start node of the returned vector is be the first element of the vector, the end node is the last element
std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {
    // Create path_found vector
  	distance = 0.0f;
    std::vector<RouteModel::Node> path_found;
  	RouteModel::Node parent;
  
    while (current_node->parent != nullptr)
    {
      	path_found.emplace_back(*current_node);
//         const RouteModel::Node parent = *(current_node->parent);
      	parent = *(current_node->parent);
        distance += current_node->distance(parent);
        current_node = current_node->parent;
    }
    path_found.emplace_back(*current_node);
    distance *= m_Model.MetricScale(); // Multiply the distance by the scale of the map to get meters
    return path_found;
}


void RoutePlanner::AStarSearch() {
    RouteModel::Node *current_node = nullptr;
    start_node->visited = true;
    open_list.emplace_back(start_node);

    while (open_list.size()>0)
    {
        // Use the NextNode() method to sort the open_list and return the next node
        current_node = NextNode();
        
        if (current_node->distance(*end_node) == 0)
        {
            //  Return the final path
            m_Model.path = ConstructFinalPath(current_node);
            return;
        }
        // Use the AddNeighbors method to add all of the neighbors of the current node to the open_list
        AddNeighbors(current_node);
    }
}
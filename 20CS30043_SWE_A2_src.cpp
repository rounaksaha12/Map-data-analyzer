/*
Name: Rounak Saha
Roll no.: 20CS30043
SWE ASSIGNMENT 2 C++
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <bits/stdc++.h>
#include <ctype.h>


#define usn unsigned
#define ll long long
#define ld long double

//libraries used for parsing from osm file
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

const ld PI = 4*atan(1.0);
const ld INF = 1e7;

inline ld toRadian(ld angle){ return angle*PI/180.0;}

//Node is equivalent to a place
class Node{
    usn ll id;
    string name;
    ld lat;
    ld lon;

public:
    Node(usn ll n, string str, ld latd, ld lond){
        id = n;
        name = str;
        lat = latd;
        lon = lond;
    }
    void set_id(usn ll n){
        id = n;
    }
    usn ll get_id(){
        return id;
    }
    string get_name(){
        return name;
    }
    ld get_lat(){
        return lat;
    }
    ld get_lon(){
        return lon;
    }
    void print_node(){
        cout<<"Node information : \n";
        cout<<"ID : "<<this->get_id()<<"\n";
        cout<<"Name : "<<this->get_name()<<"\n";
        cout<<"Latitude : "<<fixed<<setprecision(8)<<this->get_lat()<<"\n";
        cout<<"Longitude : "<<fixed<<setprecision(8)<<this->get_lon()<<"\n";
        cout<<"\n";
    }
};

class Way{
    //in this assignment ways as such donot have any importance after the information about edges has been extracted from it and the graph is created
    usn ll id;
    string name;
    vector<usn ll> path;//this will contain the list of ids of places in an order as they appear in the way
public:
    Way(usn ll n, string str){
        id = n;
        name = str;
        //path = *(new vector<usn ll>);
    }
    usn ll get_id(){
        return id;
    }
    string get_name(){
        return name;
    }
    void add_node(usn ll n){
        path.push_back(n);
    }
    void print_way(){
        cout<<"Way information:\n";
        cout<<"ID: "<<id<<"\n";
        cout<<"Name: "<<name<<"\n";
        cout<<"\n";
    }
    void print_path(){//prints the nodes in an order as they appear, thereby represents the way only
        for(auto it = path.begin();it!=path.end();++it){
            cout<<*it<<" ---> ";
        }
    }
};

class utilities{//this class contains static hepler functions used later
public:
    static ld crow_fly(Node& node1, Node& node2){//returns distance between two nodes using Haversine Formula
        ld lat1 = toRadian(node1.get_lat());
        ld lon1 = toRadian(node1.get_lon());
        ld lat2 = toRadian(node2.get_lat());
        ld lon2 = toRadian(node2.get_lon());

        ld dlon = lon2 - lon1;
        ld dlat = lat2 - lat1;
    
        ld ans = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);
        ans = 2 * asin(sqrt(ans));
        // Radius of Earth in
        // Kilometers, R = 6371
        ld R = 6371;
        ans = ans * R;
        return ans;
    }

    static bool isSubstring(const string& shrt,const string& lng){
        //checks if shrt is a substring of lng 
        int i = 0,j = 0,k = 0;
        while(i<lng.length()){
            if(tolower(lng[i]) == tolower(shrt[0])){
                k = i;
                for(j = 0;j<shrt.length() && k<lng.length();++k,++j){
                    if(tolower(lng[k])!=tolower(shrt[j])){
                        break;
                    }
                }
                if(j == shrt.length()){
                    return 1;
                }
            }
            ++i;
        }
        return 0;
    }

    static void printBanner(){
        cout<<"+-------------------------------------------------------------------------------+\n";
    }
};

class Graph{
    ll node_count; // count of number of nodes  
    ll way_count; // count of number of ways, not edges
    vector<Node> node_list; // list of nodes
    
    map< usn ll, usn ll> index_map;
    /*
    maps the id of a node to the index at which it is present in the vector node_list, searching for an id
    in the vector may be O(n), using map we can find the index at which it is present in the vector in O(logn)
    */

    vector<Way> way_list; 
    // maps the id of a way to that way object, this is not necessary though as per the requirements of the assignment

    vector< vector<pair <usn ll,ld> > > adj_list;
    /*
    adj_list[index of a node in node_list say u] is again a list of nodes having edges with u in a format pair<index of the vertex say v, u-v edge weight>
    */
   
public:
    Graph(string filename){
        // creates a graph out of an osm file with name <filename>

        //parsing the input file with rapidxml lib
        node_list = vector<Node>();
        way_list = vector<Way>();

        xml_document<> doc;

        ifstream f(filename); //taking file as inputstream
        string str;
        if(f) {
            ostringstream ss;
            ss << f.rdbuf(); // reading data
            str = ss.str();
        }
        
        doc.parse<0>(&str[0]);

        string place_identifier = "node";
        string way_identifier = "way";
        string id_identifier = "id";
        string lat_identifier = "lat";
        string lon_identifier = "lon";
        string name_identifier = "name";
        // these strings are required for comparing osm file attribute names e.g if( some_parsed_attribute->name() == id_identifier) {store some_parsed_attribute->value() after converting it to ll}
        
        char* endptr; // required for using strtold funtion used to convert char* to ld
        
        this->node_count = 0;

        xml_node<> *pRoot = doc.first_node(); // this would be the <osm> node
        for(xml_node<> *pNode = pRoot->first_node()->next_sibling(); pNode && pNode->name() == place_identifier; pNode = pNode->next_sibling()){   
            // looping through xml nodes
            usn ll n; // temporary variable to store id of a discovered node
            string str = "NULL"; // temporary string to store name of a discovered node, empty by default if any name not found
            ld latd; 
            ld lond;
            // temporary variables to store lat & long data 

            for(xml_attribute<> *pAttr = pNode->first_attribute(); pAttr ; pAttr = pAttr->next_attribute()){
                // looping through the attributes of a particular xml_node
                if(pAttr->name()==id_identifier) n = atoll(pAttr->value());
                else if(pAttr->name()==lat_identifier) latd = strtold(pAttr->value(),&endptr);
                else if(pAttr->name()==lon_identifier) lond = strtold(pAttr->value(),&endptr);
            }
            
            for(xml_node<>* cNode = pNode->first_node(); cNode; cNode = cNode->next_sibling()){
                xml_attribute<>* type_attr = cNode->first_attribute();
                if(type_attr->value() == name_identifier){
                    // looping through the child nodes of a particular xml_node, here these are the "tag" nodes
                    // the name of the node (if present) can be found in one of these child nodes
                    str = type_attr->next_attribute()->value();
                }
            }
    
            Node *newNode = new Node(n,str,latd,lond); // just a temporary node object
            
            node_list.push_back(*newNode); // add to node_list
            index_map.insert({newNode->get_id(),this->node_count}); // map the node id to its index
        
            delete newNode; // an instance of this node is already added to the vector, so deallocating the temporary newNode

            this->node_count++;
        }
        //node_count is finalized so now declare the specifications of the adj_list
        adj_list = vector< vector<pair<usn ll,ld> > >(this->node_count,vector< pair<usn ll,ld> >());
        //initializes a vector of node_count number of empty vectors of type pair<usn ll,ld>

       this->way_count = 0;

       for(xml_node<> *pNode = pRoot->first_node("way");pNode && pNode->name() == way_identifier;pNode = pNode->next_sibling()){
           // specifically loop over the xml_nodes with name "way"
           usn ll n;
           string str = "NULL"; // temp variables, name empty by default
    
           for(xml_attribute<> *pAttr = pNode->first_attribute();pAttr;pAttr = pAttr->next_attribute()){
               if(pAttr->name() == id_identifier){
                   n = atoll(pAttr->value());
                   break;
               }
           }
           for(xml_node<> *cNode = pNode->first_node("tag");cNode;cNode = cNode->next_sibling()){
               xml_attribute<> *type_attr = cNode->first_attribute();
               if(type_attr->value() == name_identifier){
                   str = type_attr->next_attribute()->value();
                   break;
               }
           }

           Way *newWay = new Way(n,str); // temporary way objext
           way_list.push_back(*newWay); // a copy is pushed into way_list
           Way thisWay = way_list[this->way_count]; //access the reference of the instance of this way in the vector
           delete newWay; //delete the temporary way object
           
           xml_node<> *prev_node = pNode->first_node("nd"); // intermediate nodes have the name = "nd" in the osm file
           thisWay.add_node(atoll(prev_node->first_attribute()->value())); // add the first node to the path of thisWay
                                                                           // path of any way element is a vector of the
                                                                           // intermediate node objects
           xml_node<> *curr_node = prev_node->next_sibling();
           string nd_identifier = "nd";
           
           while(curr_node && curr_node->name() == nd_identifier){
               usn ll prev_node_id = atoll(prev_node->first_attribute()->value());
               usn ll curr_node_id = atoll(curr_node->first_attribute()->value());

               usn ll prev_node_index = index_map.find(prev_node_id)->second;
               usn ll curr_node_index = index_map.find(curr_node_id)->second;

               thisWay.add_node(curr_node_id); //add the current node to the path of thisWay

               //we need to update the graph, add an edge between prev_node and curr_node
               //prev_node and curr_node represent two consecutive intermediate nodes of the way element
               ld wt = utilities::crow_fly(node_list[prev_node_index],node_list[curr_node_index]);
               add_edge(prev_node_index,curr_node_index,wt);
               add_edge(curr_node_index,prev_node_index,wt);// the edge must be bidirectional
               
               prev_node = curr_node;
               curr_node = curr_node->next_sibling();
           }
           this->way_count++;
       }
       cout<<"+----------------------------------------------+\n";
       cout<<"GRAPH CONSTRUCTED SUCCESSFULLY FROM "<<filename<<"\n";
       cout<<"Number of nodes discovered : "<<node_count<<"\n";
       cout<<"Number of ways discovered : "<<way_count<<"\n";
       cout<<"+----------------------------------------------+\n";
    }

    void add_edge(usn ll u, usn ll v, ld wt){
        adj_list[u].push_back({v,wt});
    }

    void print_all_nodes(){
        for(auto it = node_list.begin();it!=node_list.end();++it){
            it->print_node();
        }
    }

    usn ll get_node_count(){
        return node_count;
    }

    usn ll get_way_count(){
        return way_count;
    }

    void print_shortest_path(vector<usn ll> parent, usn ll dest){// Recursive function to print the path using the information
                                                                 // of parent of a node in the shortest path tree
        if(parent[dest] == dest){
            cout<<"Node id: "<<node_list[dest].get_id()<<" ( Name: "<<node_list[dest].get_name()<<", Latitude: "<<fixed<<setprecision(8)<<node_list[dest].get_lat()<<", Longitude: "<<fixed<<setprecision(8)<<node_list[dest].get_lon()<<")\n";
            return;
        }
        print_shortest_path(parent,parent[dest]);
        cout<<"Node id: "<<node_list[dest].get_id()<<" ( Name: "<<node_list[dest].get_name()<<", Latitude: "<<fixed<<setprecision(8)<<node_list[dest].get_lat()<<", Longitude: "<<fixed<<setprecision(8)<<node_list[dest].get_lon()<<")\n";
    }

    void shortest_path(usn ll src_id){ // Dijkstra's single source shortest path finding

        if(index_map.find(src_id) == index_map.end()){
            cout<<"ERROR: INVALID ID :(\n";
            return;
        }

        usn ll src = index_map.find(src_id)->second;
        set< pair<ld,usn ll> > dst; // set keeps pair elements stored in increasing order of the first elements
                                    // this set stores nodes (whose shortest dist is not yet finalized) in a format
                                    // pair<shortest dist discovered so far, index of the node> so that the topmost
                                    // element has the shortest distance 
        vector<ld> min_dst(node_count,INF); // array with shortest path values
        vector<usn ll> parent(node_count,src); // array with inex of parent in shortest path tree
        vector<bool> is_found(node_count,false); // whether shortest path has been finalized

        // add the source node
        min_dst[src] = 0.0;
        parent[src] = src;
        is_found[src] = true;
        dst.insert({0.0,src});

        // pop from the set, add it to the list of nodes whose shortest path has been finalized and update
        // shortest path discovered so far of its neighbours till the set is empty
        while(!dst.empty()){
            auto temp = dst.begin();
            usn ll u = temp->second;
            ld u_dist = min_dst[u];
            dst.erase(dst.begin());
            if(u!=src && is_found[u]) continue;
            is_found[u] = true; // add u to the list of nodes whose shortest path has been finalized
            for(auto it = adj_list[u].begin();it!=adj_list[u].end();++it){
                usn ll v = it->first;
                ld u_v_wt = it->second;
                ld v_dist = min_dst[v];
                if(is_found[v] == false && v_dist>u_dist+u_v_wt){ // update shortest path discovered so far of v
                    if(min_dst[v]!=INF){
                        dst.erase(dst.find({v_dist,v}));
                    }
                    min_dst[v] = u_dist+u_v_wt;
                    parent[v] = u;
                    dst.insert({min_dst[v],v});
                }
            }
        }
        
        cout<<"Shortest paths of all nodes from given source have been computed.\n";
        cout<<"\n";

        char response = 'Y';
        usn ll dest_id;
        // since now shortest path to all nodes from src is available to us now, we ask the user to enter as many destinations as he wants
        // doing so would save a great deal of computation!
        while(response != 'N'){
            cout<<">> Enter a destination id: ";
            cin>>dest_id;
            cout<<"\n";
            if(index_map.find(dest_id) == index_map.end()){
                cout<<"ERROR: INVALID ID :( TRY AGAIN\n";
                continue;
            }
            if(min_dst[index_map.find(dest_id)->second] == INF){
                cout<<"No path found\n";
                cout<<"\n";
            }
            else{
                cout<<"Follow the nodes listed below in that sequence for shortest path to your destination: \n";
                cout<<"------------------------------------------------------------------------------------- \n";
                print_shortest_path(parent,index_map.find(dest_id)->second);
                cout<<"\n";
                cout<<"++ Distance traversed in the above path: "<<min_dst[index_map.find(dest_id)->second]<<" kilometer(s)\n";
                cout<<"\n";
            }
            cout<<"Want to enter another destination id ? (Y/N) : ";
            cin>>response;
            cout<<"\n";
        }

    }

    void search_by_substr_node(){
        string input_str;
        cout<<"Enter a part of name: ";
        getline(cin,input_str);
        cout<<"\n";
        int count = 0;
        for(auto it = node_list.begin();it!=node_list.end();++it){
            // loops over all the nodes
            if(it->get_name() == "NULL") continue;
            if(utilities::isSubstring(input_str,it->get_name())){
                cout<<"Match number ( "<<count+1<<" )\n";
                it->print_node();
                count++;
            }
        }
        cout<<"\n";
        cout<<count<<" matches found\n";
        cout<<"\n";
    }

    void search_by_substr_way(){
        string input_str;
        cout<<"Enter a part of name: ";
        getline(cin,input_str);
        cout<<"\n";
        int count = 0;
        for(auto it = way_list.begin();it!=way_list.end();++it){
            // loops over all the nodes
            if(it->get_name() == "NULL") continue;
            if(utilities::isSubstring(input_str,it->get_name())){
                cout<<"Match number ( "<<count+1<<" )\n";
                it->print_way();
                count++;
            }
        }
        cout<<"\n";
        cout<<count<<" matches found\n";
        cout<<"\n";
    }

    void k_nearest_nodes(){
        usn ll k;
        usn ll centre_id;
        cout<<">> Enter id of node: ";
        cin>>centre_id;
        if(index_map.find(centre_id) == index_map.end()){
            cout<<"\nERROR: INVALID ID :(\n";
            return;
        }
        cout<<">> Enter value of k: ";
        cin>>k;
        cout<<"\n";
        if(k<=0){
            cout<<"INVALID INPUT :( TRY AGAIN\n";
            cout<<"\n";
            return;
        }
        Node centre = node_list[index_map.find(centre_id)->second];
        priority_queue< pair<ld,usn ll>, vector<pair<ld,usn ll> >, greater<pair<ld,usn ll> > > pq;
        // push a node into the min heap with format pair<distance from the concerned node,index of it in node_list>
        // pop k times to get k nearest neighbours
        ld curr_dist;
        for(auto it = node_list.begin();it!=node_list.end();++it){
            if(it->get_id() == centre_id) continue;
            curr_dist = utilities::crow_fly(*it,centre);
            pq.push({curr_dist,it-node_list.begin()});
        }
        usn ll count = 0;
        cout<<k<<" nearest neighbours are: \n";
        cout<<"-----------------------------\n";
        while(!pq.empty() && count<k){
            cout<<"( "<<count+1<<" ) Distance: "<<pq.top().first<<"\n";
            node_list[pq.top().second].print_node();
            pq.pop();
            count++;
        }
    }
};

int main(){
    string filename = "map.osm"; // file name goes here
    Graph g = Graph(filename);
    //g.print_all_nodes();
    char response = 'Y';
    int choice;
    while(response!='N'){ // menu based interactive text interface
        cout<<"\n";
        cout<<"++ CHOOSE FROM THE FOLLOWING OPTIONS ++\n";
        cout<<"\n";
        cout<<"   To search nodes by name/part of name     : Press 1\n";
        cout<<"   To search ways by name/part of name      : Press 2\n";
        cout<<"   To find k-closest neighbours of a node   : Press 3\n";
        cout<<"   To find shortest path from a source node : Press 4\n";
        cout<<"\n";
        cout<<">> Enter your response here: ";
        cin>>choice;
        cout<<"\n";
        cin.ignore(numeric_limits<streamsize>::max(),'\n'); //consumes the newline
        switch(choice){
            case 1:{
                g.search_by_substr_node();
                break;
            }
            case 2:{
                g.search_by_substr_way();
                break;
            }
            case 3:{
                g.k_nearest_nodes();
                break;
            }
            case 4:{
                usn ll src_id;
                cout<<">> Enter id of source node: ";
                cin>>src_id;
                cout<<"\n";
                g.shortest_path(src_id);
                break;
            }
        }
        cout<<"Do you want to continue: ";
        cin>>response;
        cout<<"\n";
        utilities::printBanner();
    }
    return 0;
}

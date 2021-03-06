#include "TopologicalFeatures.hpp"
#include <fstream>
#include "constants.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include "utl.h"

namespace contourtree {

TopologicalFeatures::TopologicalFeatures() { }

void TopologicalFeatures::loadData(std::string dataLocation, bool partition) {
    ctdata = ContourTreeData();
    ctdata.loadBinFile(dataLocation);

    // read order file
    // read meta data
	std::ifstream ip(dataLocation + ".order.dat");
	if (!ip.is_open()) {
		std::cout << "could not read file" << dataLocation + ".order.dat";
    }

//QTextStream text(&ip);
	std::string line;
	std::getline(ip, line);
	std::stringstream ss(line);
	int orderSize;
	ss >> orderSize;

 ip.close();

    order.resize(orderSize);
    wts.resize(orderSize);

    std::string binFile = dataLocation + ".order.bin";
    std::ifstream bin(binFile, std::ios::binary);
    bin.read((char *)order.data(),order.size() * sizeof(uint32_t));
    bin.read((char *)wts.data(),wts.size() * sizeof(float));
    bin.close();

    if(partition) {
        sim.setInput(&ctdata);
        sim.simplify(order,1,0,wts);
    }
}

void TopologicalFeatures::addFeature(SimplifyCT &sim, uint32_t bno, std::vector<Feature> &features, std::set<size_t> &featureSet) {
    Branch b1 = sim.branches.at(bno);
    Feature f;
    f.from = ctdata.nodeVerts[b1.from];
    f.to = ctdata.nodeVerts[b1.from];

    std::deque<size_t> queue;
    queue.push_back(bno);
    while(queue.size() > 0) {
        size_t b = queue.front();
        queue.pop_front();
        ENSURES(!(b != bno && featureSet.count(b)!=0));
        featureSet.insert(b);
        Branch br = sim.branches.at(b);
        f.arcs.insert(f.arcs.end(),br.arcs.data(), br.arcs.data()+br.arcs.size());
        for(int i = 0;i < br.children.size();i ++) {
            uint32_t bc = br.children.at(i);
            queue.push_back(bc);
        }
    }
    features.push_back(f);
}

std::vector<Feature> TopologicalFeatures::getFeatures(int topk, float th, float secondary) {
    SimplifyCT sim;
    sim.setInput(&ctdata);

    sim.simplify(order,topk,th,wts);
    std::vector<Feature> features;

    std::set<size_t> featureSet;
    for(size_t _i = 0;_i < order.size();_i ++) {
        size_t i = order.size() - _i - 1;
        if(sim.removed[order[i]]) {
            break;
        }
		featureSet.insert(order[i]);
    }
    size_t pos = 0;
    for(size_t _i = 0;_i < order.size();_i ++) {
        size_t i = order.size() - _i - 1;
        if(sim.removed[order[i]]) {
            pos = _i;
            break;
        }
        addFeature(sim,order[i],features,featureSet);
    }

    for(size_t _i = pos;_i < order.size();_i ++) {
        size_t i = order.size() - _i - 1;
        if(featureSet.count(order[i])!=0) {
            continue;
        }
        uint32_t bno = order[i];
		featureSet.insert(bno);
        Branch br = sim.branches.at(bno);
        uint32_t from = br.from;
        uint32_t to = br.to;
        float per = ctdata.fnVals[to] - ctdata.fnVals[from];
        // TODO make any leaf?
        if(ctdata.type[to] == MAXIMUM && per >= secondary) {
            addFeature(sim,bno,features,featureSet);
        }
    }
    return features;
}

std::vector<Feature> TopologicalFeatures::getPartitionedExtremaFeatures(int topk, float th) {
    std::vector<Feature> features;

    std::set<size_t> featureSet;
    if(topk == -1) {
        topk = 0;
        for(int i = order.size() - 1;i >= 0 ;i --) {
            if(wts[i] > th) {
                topk ++;
                featureSet.insert(order[i]);
            } else {
                break;
            }
        }
    }
    if(topk == 0) topk = 1;

    for(int _i = 0;_i < topk;_i ++) {
        size_t i = order.size() - _i - 1;
        Branch b1 = sim.branches.at(order[i]);
        Feature f;
        f.from = ctdata.nodeVerts[b1.from];
        f.to = ctdata.nodeVerts[b1.to];

        size_t bno = order[i];
        std::deque<size_t> queue;
        queue.push_back(bno);
        while(queue.size() > 0) {
            size_t b = queue.front();
            queue.pop_front();
            if(b != bno && featureSet.count(b) !=0) {
                continue;
            }
            Branch br = sim.branches.at(b);
            f.arcs.insert(f.arcs.end(),br.arcs.data(), br.arcs.data()+br.arcs.size());
            for(int i = 0;i < br.children.size();i ++) {
                int bc = br.children.at(i);
                queue.push_back(bc);
            }
        }
        features.push_back(f);
    }
    return features;
}

std::vector<Feature> TopologicalFeatures::getArcFeatures(int topk, float th) {
    SimplifyCT sim;
    sim.setInput(&ctdata);

    sim.simplify(order,topk,th,wts);

    std::vector<Feature> features;
    std::set<size_t> featureSet;
    for(size_t _i = 0;_i < sim.branches.size();_i ++) {
        if(sim.removed[_i]) {
            continue;
        }
        featureSet.insert( _i);
    }
    for(size_t _i = 0;_i < sim.branches.size();_i ++) {
        size_t i = _i;
        if(sim.removed[i]) {
            continue;
        }
        Branch b1 = sim.branches.at(i);
        Feature f;
        f.from = ctdata.nodeVerts[b1.from];
        f.to = ctdata.nodeVerts[b1.to];

        size_t bno = i;
        std::deque<size_t> queue;
        queue.push_back(bno);
        while(queue.size() > 0) {
            size_t b = queue.front();
            queue.pop_front();
            ENSURES(!(b != bno && featureSet.count(b) !=0));
            Branch br = sim.branches.at(b);
            f.arcs.insert(f.arcs.end(),br.arcs.data(), br.arcs.data()+br.arcs.size());
            for(int i = 0;i < br.children.size();i ++) {
                int bc = br.children.at(i);
                queue.push_back(bc);
            }
        }
        features.push_back(f);
    }
    return features;
}

}

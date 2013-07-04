#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "Vertex.h"
#include "Block.h"
#include "MapParser.h"
#include "rapidxml.hpp"

using namespace rapidxml;

MapParser::MapParser() : m_width(0), m_height(0), m_gids(0)
{}

MapParser::~MapParser()
{}

void MapParser::init(ID3D10Device *dev)
{
	m_d3dDevice = dev;
}

void MapParser::loadMap(std::string fileName)
{
	//Read the content of the file
	try
	{
		std::ifstream file(fileName);
		std::istreambuf_iterator<char> eos;
		std::istreambuf_iterator<char> iit (file);
		
		while(iit!=eos)
			m_xml+=*iit++;

		file.close();
	}
	catch (std::ifstream::failure e)
	{
		std::cout<<"Map read error"<<std::endl;
	}
}

std::string& MapParser::getXml()
{
	return m_xml;
}

std::vector<DWORD>& MapParser::getGids()
{
	return m_gids;
}

DWORD& MapParser::getWidth()
{
	return m_width;
}

DWORD& MapParser::getHeight()
{
	return m_height;
}

BlockProperties& MapParser::getBlockProperties()
{
	return m_blockProperties;
}

//Process one gid
void MapParser::processGid(xml_node<>* XMLNode)
{
	//Get definition of the gid
	DWORD gid = atoi(XMLNode->first_attribute("firstgid")->value());
	XMLNode = XMLNode->first_node("image");
	string path = XMLNode->first_attribute("source")->value();
	m_blockProperties.setPathToTexture(gid, path);
	#if defined( DEBUG ) || defined( _DEBUG )
	std::cout<<"\tGID("<<gid<<"): "<<m_blockProperties.getPathToTexture(gid)<<std::endl;
	#endif
	
	XMLNode = XMLNode->next_sibling("tileset");
}

//Get everything we need from XML
void MapParser::parseMap(const std::string& xml)
{
	#if defined( DEBUG ) || defined( _DEBUG )
	std::cout<<"parseMap()"<<std::endl;
	#endif

	std::vector<char> xmlCopy(xml.begin(), xml.end());
	xmlCopy.push_back('\0');

	xml_document<> doc;	//Create instance for parsing

	doc.parse<parse_declaration_node | parse_no_data_nodes>(&xmlCopy[0]);
	
	xml_node<>* curNode = doc.first_node("map")->first_node("tileset");

	//Process each gid defined in tileset
	while(curNode)
	{
		processGid(curNode);
		curNode = curNode->next_sibling("tileset");
	}

	//Get metric data of the map
	curNode = doc.first_node("map")->first_node("layer");
	std::string width = curNode->first_attribute("width")->value();
	std::string height = curNode->first_attribute("height")->value();
	#if defined( DEBUG ) || defined( _DEBUG )
	std::cout<<"Size: "<<width<<" * "<<height<<std::endl;
	#endif

	m_width = atoi(width.c_str());
	m_height = atoi(height.c_str());
	
	curNode = doc.first_node("map")->first_node("layer")->first_node("data")->first_node("tile");

	//Parse gids
	while(curNode)
	{
		m_gids.push_back(atoi(curNode->first_attribute("gid")->value()));
		curNode = curNode->next_sibling("tile");
	}
}

//Draw each map block
bool MapParser::generateMap(vector<DWORD>& gids, BlockProperties& blockInfo)
{
	DWORD index = 0;
	vector<Vertex> vertex;
	/*#if defined( DEBUG ) || defined( _DEBUG )
	cout<<"height: "<<m_height<<" width: "<<m_width<<endl;
	#endif*/
	Block block;

	//Save XML data to each map block
	for(float i=0; i<m_height; i++)
	{
		for(float j=0; j<m_width; j++)
		{
			block.parseData(m_d3dDevice, i, j, m_blockProperties.getPathToTexture(m_gids[index]));
			index++;
			//cout<<"INDEX: "<<index<<endl;
			block.draw();			
		}
	}
	
	return true;
}
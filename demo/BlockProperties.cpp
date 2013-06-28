#include <vector>
#include <string>
#include <iostream>
#include "BlockProperties.h"

using namespace std;

BlockProperties::BlockProperties()
{}

BlockProperties::~BlockProperties()
{}

string BlockProperties::getName()
{
	return m_name;
}

void BlockProperties::setName(string name)
{
	m_name = name;
}

void BlockProperties::setPathToTexture(int gid, string pathToTexture)
{
	m_textureOfGid[gid] = pathToTexture;
}

string BlockProperties::getPathToTexture(int gid)
{
	//cout<<"TEXTURE TEST: "<<m_textureOfGid[gid]<<endl;
	return m_textureOfGid[gid];
}
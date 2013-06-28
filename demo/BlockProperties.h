#ifndef BLOCKPROPERTIES
#define BLOCKPROPERTIES

#include <string>
#include <map>

using namespace std;

class BlockProperties
{
public:
	BlockProperties();
	~BlockProperties();
	string getName();
	void setName(string name);
	void setPathToTexture(int gid, string pathToTexture);
	string getPathToTexture(int gid);
private:
	string m_name;
	map<int, string> m_textureOfGid;
};

#endif //BLOCKPROPERTIES
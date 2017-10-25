#include<iostream>
#include<string>
using namespace std;
string quadconvert(string s)
{
	int i=0;
	string quad,temp;
	while(i<8)
	{
		temp=s.substr(i,2);
		if(temp=="00")
			quad.append("0");
		else if(temp=="01")
			quad.append("1");
		else if(temp=="10")
			quad.append("2");
		else if(temp=="11")
			quad.append("3");
		i=i+2;
	}
	return quad;
}

#ifndef SH_PATH_H
#define SH_PATH_H

#include <list>
#include <string>
#include <stack>
namespace Slnady
{

	struct Path
	{
		std::list<std::string> dirs;
		std::stack<std::string> poppedDirs;
		size_t pos;
		bool isAbsolute;
		Path(const std::string& strArg) : isAbsolute(false)
		{
			std::string str = strArg;
			if (str[1] == ':') isAbsolute = true;
			while (str[0] == '\\')
			{
				str.erase(0, 1);
			}
			pos = 0;
			size_t slash = 0;
			size_t lastSlash = 0;
			while (slash != std::string::npos)
			{
				slash = str.find('\\', slash);
				if (slash != std::string::npos)
				{
					dirs.push_back(str.substr(lastSlash, slash - lastSlash));
					lastSlash = ++slash;
					pos++;
				}
				else if ((str.size() - lastSlash) > 1)
				{
					dirs.push_back(str.substr(lastSlash, str.size() - lastSlash));
					pos++;
				}
			}
		}
		Path() : isAbsolute(false), pos(0), dirs() {}

		Path& operator+=(const std::string& dir)
		{
			dirs.push_back(dir);
			pos++;
			return *this;
		}
		Path& operator+=(const Path& other)
		{
			for (auto it = other.dirs.begin(); it != other.dirs.end(); it++)
			{
				dirs.push_back(*it);
			}
			pos += other.dirs.size();
			return *this;
		}

		Path operator+(const Path& other)
		{
			Path result{};
			for (auto it = dirs.begin(); it != dirs.end(); it++)
			{
				result.dirs.push_back(*it);
			}
			for (auto it = other.dirs.begin(); it != other.dirs.end(); it++)
			{
				result.dirs.push_back(*it);
			}
			
			return result;
		}
		void back()
		{
			if (dirs.size())
			{
				poppedDirs.push(dirs.back());
				dirs.pop_back();
			}
		}
		void forward()
		{
			if (poppedDirs.size())
			{
				dirs.push_back(poppedDirs.top());
				poppedDirs.pop();
			}
		}

		std::string getString()
		{
			std::string res;
			if (dirs.size())
			{
				auto it = dirs.begin();
				auto lastIt = dirs.end();
				size_t tempPos = 1;
				while ((it != lastIt) && tempPos <= dirs.size())
				{
					res += *it;
					if (tempPos != dirs.size())
					{
						res += '\\';
					}
					it++;
					tempPos++;
				}
			}
			return res;
		}

		//This assumes both paths are absolute
		Path getRelativeTo(const Path& other)
		{
			Path res{};
			auto thisBegin = dirs.begin();
			auto thisEnd = dirs.end();
			auto otherBegin = other.dirs.begin();
			auto otherEnd = other.dirs.end();
			size_t maxMatchPos = 0;
			while (thisBegin != thisEnd)
			{
				if (otherBegin != otherEnd)
				{
					if (!(*thisBegin).compare(*otherBegin)) maxMatchPos++;
					otherBegin++;
					
				}
				thisBegin++;
			}
			if (maxMatchPos > 0)
			{
				size_t dif = other.dirs.size() - maxMatchPos;

				for (size_t tmp = 0; tmp < dif; tmp++)
				{
					res += "..";
				}
				thisBegin = dirs.begin();
				for (size_t i = 0; i < maxMatchPos; i++)
				{
					thisBegin++;
				}
				
				while (thisBegin != thisEnd)
				{
					res += *thisBegin;
					thisBegin++;
				}
			}

			return res;
		}
	};

}
#endif



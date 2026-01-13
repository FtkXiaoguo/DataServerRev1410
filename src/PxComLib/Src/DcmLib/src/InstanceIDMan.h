
//////////////////////////////////////////////////////////////////////
 

#if !defined(_INSTANCE_ID_MAN_H_)
#define _INSTANCE_ID_MAN_H_
 
#include <list>
#include <vector>
#include <map>

using namespace std;

class ConfigEntry
{
public:
	ConfigEntry( ){ ;
	}
	ConfigEntry(string val){
		m_string_Config = val;
	}
	ConfigEntry(bool val){
		m_bool_Config = val;
	}
	ConfigEntry(long int val){
		m_long_Config = val;
	}
	ConfigEntry(int val){
		m_int_Config = val;
	}
//
	string m_string_Config;
	bool m_bool_Config;
	long m_long_Config;
	int m_int_Config;
};
typedef map <int , ConfigEntry> ConfigMap;
typedef ConfigMap::iterator ConfigIterator;
typedef pair< int, ConfigEntry > theConfigPair;

#if 0
template <class T> 
class InstanceEntry
{
public:
	InstanceEntry(T obj)
	{ 
		m_ObjPtr = obj ;
		m_Empty  = false;
	};
	T m_ObjPtr;
	bool m_Empty;
};
#endif

template <class T>
class InstanceIDManage
{
public:
//typedef  InstanceEntry<T>  InstanceEntryType ;
typedef  map<int,T> InstanceMap;
//typedef map<int,T>::iterator InstanceIterator;

	InstanceIDManage()
	{

		 
	}
	void clearAll() 
	{
		DcmLibMTLock lock;
		m_InstanceMap.clear();
	}
	bool registerInstance(int id,T obj)
	{
		DcmLibMTLock lock;
		m_InstanceMap[id] = obj;
#if 0
		int ret_id = -1;
		InstanceEntryType  new_Entry(obj);

		int old_id = findEmptyID();
		if(old_id<0){
			m_InstanceList.push_back(new_Entry);
			ret_id = m_InstanceList.size()-1;
		}else{
			m_InstanceList[old_id] = new_Entry;
			ret_id = old_id;
		}
		return ret_id;
#endif
		return true;
	}

	T removeInstance(int id)
	{
		DcmLibMTLock lock;
		T ret_val = 0;
		map<int,T>::iterator it = m_InstanceMap.find(id);
		if(it == m_InstanceMap.end() ){
			return ret_val;
		}else{
			ret_val = it->second;
			m_InstanceMap.erase(it);
			return ret_val;
		}
#if 0
		if( (id<0) || (id>=m_InstanceList.size())){
			return false;
		}
		InstanceEntryType Instance_entry = m_InstanceList[id];
		Instance_entry.m_Empty = true; 
		T ret_ptr = Instance_entry.m_ObjPtr;
		Instance_entry.m_ObjPtr = 0;
		m_InstanceList[id] = Instance_entry;
		return ret_ptr;
#endif
	}
	T getInstance(int id)
	{
		DcmLibMTLock lock;
		T ret_val = 0;
		map<int,T>::iterator it = m_InstanceMap.find(id);
		if(it == m_InstanceMap.end() ){
			return ret_val;
		}else{
			ret_val = it->second;
			return ret_val;
		}

#if 0
		T ret_val = 0;
		vector<InstanceEntryType >::iterator it = m_InstanceList.begin();  
		 
		if( (id<0) || (id>=m_InstanceList.size())){
			return 0;
		}

		InstanceEntryType Instance_entry = m_InstanceList[id];
		ret_val = Instance_entry.m_ObjPtr ;
		return ret_val;
#endif
	}

#if 0
	bool setInstance(int id,T obj)
	{
		if( (id<0) || (id>=m_InstanceList.size())){
			return false;
		}

		m_InstanceList[id] = obj;
		 
		return true;
	}
#endif

//	vector<InstanceEntryType >::iterator begin() { return m_InstanceList.begin();};
//	vector<InstanceEntryType >::iterator end() { return m_InstanceList.end();};
	
 	int getSize(){ return m_InstanceMap.size();};
	
 	T getItem(int index) { 
		DcmLibMTLock lock;
		int map_size = getSize();
		map<int,T>::iterator it = m_InstanceMap.begin();
		if(index<1){
			return it->second;
		}
		for(int i=0;i<index;i++){
			it++;
		}
		return it->second;
	 
	};

	void removeAllInstance()
	{
		DcmLibMTLock lock;
		map<int,T>::iterator it = m_InstanceMap.begin();
		while(it!=m_InstanceMap.end()){
			(it->second)->Delete();
			delete it->second; 
			it++;
		}
		clearAll();
	}
	 
protected:

	InstanceMap m_InstanceMap; 

	//

#if 0
	int findEmptyID()
	{
		int ret_id = -1;
		int size = m_InstanceList.size();
		int list_count = -1;
		 vector<InstanceEntryType >::iterator it = m_InstanceList.begin();  
		while( it != m_InstanceList.end() )  //  
		{
			list_count++;
	
			if(it->m_Empty){
				//recycle use it
				ret_id = list_count;
				break;
			}
			it++;
		}
		return  ret_id;

	}
#endif
};

#endif // !defined(_INSTANCE_ID_MAN_H_)
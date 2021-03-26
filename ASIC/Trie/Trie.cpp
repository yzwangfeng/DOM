
#include <set>
#include<map>
#include <queue>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include<vector>
using namespace std;
map<char, int> ys;
struct Tre{
    int to[2];
    int tag;
	int match;
	Tre(){
        to[1]=0;
        to[0]=0;
        tag=0;
		match=0;
	}
}Trie[500005];
map<int, double> area_map;
int f_trie[513], val_trie[513];//f为排列顺序，val为输入的值
double areanow;//某一个门的面积
char s_trie[513];//输入的表达式
int nstr[513];//真值表
int cnt_trie,my_cnt_trie,k_trie;
bool expression_bool();
bool term_bool();
bool factor_bool();
ifstream fffin("111.txt", ios::in); 
// term expression factor 三个通过递归的方式求解表达式的值
bool term_bool(){
	char op=s_trie[my_cnt_trie];
	if(op=='!'){
		my_cnt_trie++;
		return !factor_bool();
	}
    return factor_bool();

}
bool expression_bool(){
	bool result=term_bool();
	bool more=true;
	while(more){
		char op=s_trie[my_cnt_trie];
		if(op==' '||op=='+'||op=='^'){
			my_cnt_trie++;
			bool value=term_bool();
			if(op=='^')result=result^value;
			else if(op==' ') result=result&value;
			else result=result|value;
		}else{
			more=false;
		}
	}
 
	return result;
}
bool factor_bool(){
	bool result=0;
	char c=s_trie[my_cnt_trie];
	if(c=='('){
		my_cnt_trie++;
		result=expression_bool();
		my_cnt_trie++;
	}
    else if(c>='A'&&c<='Z'){
		result=val_trie[f_trie[ys[c]]];
		my_cnt_trie++;
	}
	else if(c=='!')result=term_bool();
	return result;
}
void ins_trie(int s_trie[],int match){//将得到的真值表插入到Trie树中
    int x=0,i=0;
    while(i<(1<<k_trie)){
		int j=s_trie[i];
		if(!Trie[x].to[j])Trie[x].to[j]=++cnt_trie,x=cnt_trie;
		else x=Trie[x].to[j];
		i++;
		}
		area_map[x] = areanow;//该真值表对应的门的面积
		cout <<"###"<<x << ' ' << areanow <<' '<<match<< '\n';
		Trie[x].tag = 1;//表示该结点对应了某一个特定的门
		Trie[x].match = (1 << match);//双输出门匹配
}
int getnum(char* s_trie,int len){//从表达式中得到输入个数，并分别标号
	int jsq = 0;
	for (int i = 0; i < len;i++){
		if(s_trie[i]>='A'&&s_trie[i]<='Z'){
			if(!ys[s_trie[i]])
				ys[s_trie[i]] = ++jsq;
		}
	}
	return jsq;
}
void ins(int match){
	memset(s_trie,0,sizeof(s_trie));
	ys.clear();
	fffin.getline(s_trie, 256);
	if (strlen(s_trie) <= 1)
		fffin.getline(s_trie, 256);
	int len = strlen(s_trie);
	k_trie = getnum(s_trie, len);
	cout << k_trie<<' '<<s_trie<<' '<<areanow << '\n';
	int num=1;
    for(int i=1;i<=k_trie;i++)num*=i,f_trie[i]=i;
    
    for(int i=0;i<num;i++){
        for(int t=0;t<(1<<k_trie);t++){
            for(int j=0;j<k_trie;j++)val_trie[j+1]=(t>>j)&(1);//求各个输入的值
			my_cnt_trie = 0;
			nstr[t]=expression_bool();//求某一全排列下的真值表
		}
		for(int j=0;j<(1<<k_trie);j++)cout<<nstr[j]<<' ';
       cout<<"\n";

        ins_trie(nstr,match);//将真值表插入到trie树中
        next_permutation(f_trie+1,f_trie+k_trie+1);//求全排列
        
    }
}
void presettings(){
	int ff = 0;
	int jsq = 0;
	while(1){
		fffin >>ff>> areanow;
		if(ff==0)
			break;
		if(ff==1)ins(0);
		else ins(++jsq),ins(jsq);
	}
}
int main(){
    presettings();
}
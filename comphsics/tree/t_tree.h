#ifndef COMPHSICS_ADT_T_TREE_H
#define COMPHSICS_ADT_T_TREE_H


#include "comphsics/tree/b_tree.h"
#include <set>

namespace comphsics{
	namespace tree{
		
		template<typename DataType,typename NodeType=node::t_node<DataType>>
		class t_tree:public b_tree<DataType,NodeType>{
			using basic_type=b_tree<DataType,NodeType>;

			// prohibits all public ordering methods.
			using basic_type::level_order;
			using basic_type::pre_order;
			using basic_type::pre_order_r;
			using basic_type::post_order;
			using basic_type::post_order_r;
			using basic_type::in_order;
			using basic_type::in_order_r;

			enum class THREAD_KIND{
				UNTHREADED,THREAD_PRE,THREAD_IN,THREAD_POST
			};
		
		
			TREE_TRAITS(NodeType)
		private:
			// thread tree unresolved problem:
			// 1): find prior of a node in pre-thread tree.
			// 2): find after of a node in post-thread tree.
			enum THREAD_KIND _kind;
			// stores all nodes that have threads(including null node) to cache.
			std::set<node_pointer> _thread_nodes; 

			explicit t_tree(node_pointer t)
				:b_tree<DataType,NodeType>(t){
					_kind=THREAD_KIND::UNTHREADED;
				}
			explicit t_tree(std::nullptr_t):b_tree<DataType,NodeType>(nullptr){
				_kind=THREAD_KIND::UNTHREADED;
			}
			// methods to build threads.
			// The second is passed by reference! 
			void tree_pre_thread_impl(node_pointer node
					,node_pointer& pre_node){
				if (!node->left_child) {
					node->left_is_thread = true;
					node->left_child= pre_node;
					_thread_nodes.insert(node);
				}
				if(pre_node&&!pre_node->right_child){
					pre_node->right_child=node;
					pre_node->right_is_thread=true;
					_thread_nodes.insert(pre_node);
				}
				pre_node=node;
				if (!node->left_is_thread && node->left_child) {
					tree_pre_thread_impl(node->left_child,pre_node);
				}
				if (!node->right_is_thread && node->right_child) {
					tree_pre_thread_impl(node->right_child,pre_node);
				}
			}

			void tree_in_thread_impl(node_pointer node
					,node_pointer& pre_node) {
				if (!node->left_is_thread && node->left_child) {
					tree_in_thread_impl(node->left_child,pre_node);
				}
				if (!node->left_child) {
					node->left_is_thread = true;
					node->left_child = pre_node;
					_thread_nodes.insert(node);
				}
				if(pre_node&&!pre_node->right_child){
					pre_node->right_child=node;
					pre_node->right_is_thread=true;
					_thread_nodes.insert(pre_node);
				}
				pre_node=node;
				if (!node->right_is_thread && node->right_child) {
					tree_in_thread_impl(node->right_child,pre_node);
				}
			}

			void tree_post_thread_impl(node_pointer node
					,node_pointer& pre_node){
				if (!node->left_is_thread && node->left_child) {
					tree_post_thread_impl(node->left_child,pre_node);
				}
				if (!node->right_is_thread && node->right_child) {
					tree_post_thread_impl(node->right_child,pre_node);
				}
				if (!node->left_child) {
					node->left_is_thread = true;
					node->left_child = pre_node;
					_thread_nodes.insert(node);
				}
				if(pre_node&&!pre_node->right_child){
					pre_node->right_child=node;
					pre_node->right_is_thread=true;
					_thread_nodes.insert(pre_node);
				}
				pre_node=node;
			}
		public:
			node_pointer pre_first(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_PRE&&"Cannot get pre first in pre thread tree!");
				return node;
			}
			node_pointer pre_last(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_PRE&&"Cannot get pre last in pre thread tree!");
				node_pointer tmp=node;
				while(!tmp->right_is_thread&&tmp->right_child){
					tmp=tmp->right_child;
				}
				return tmp;
			}
			
			node_pointer pre_after(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_PRE&&"Cannot get pre after in pre thread tree!");
				if(node->right_is_thread){
					return node->right_child;
				}else if(node->left_child){
					return node->left_child;
				}else{
					return node->right_child;
				}
			}
			
			
			node_pointer in_first(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_IN&&"Cannot get in first in in-thread tree!");
				node_pointer  tmp=node;
				while(!tmp->left_is_thread&&tmp->left_child){
					tmp=tmp->left_child;
				}
				return tmp;
			}
			node_pointer in_last(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_IN&&"Cannot get in last in in-thread tree!");
				node_pointer tmp=node;
				while(!tmp->right_is_thread&&tmp->right_child){
					tmp=tmp->right_child;
				}
				return tmp;
			}

			node_pointer in_prior(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_IN&&"Cannot get in prior in in-thread tree!");
				node_pointer tmp=node->left_child;
				if(!node->left_is_thread&&node->left_child) {
					tmp=in_last(tmp);
				}
				return tmp;
			}

			node_pointer in_after(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_IN&&"Cannot get in after in in-thread tree!");
				node_pointer tmp=node->right_child;
				if(!node->right_is_thread&&node->right_child) {
					tmp=in_first(tmp);
				}
				return tmp;
			}
			
			node_pointer post_first(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_POST&&"Cannot get post first in post-thread tree!");
				node_pointer  tmp=node;
				while(!tmp->left_is_thread&&tmp->left_child){
					tmp=tmp->left_child;
				}
				return tmp;
			}
			node_pointer post_last(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_POST&&"Cannot get post last in post-thread tree!");
				return node;
			}
			node_pointer post_prior(node_pointer node)const{
				assert(_kind==THREAD_KIND::THREAD_POST&&"Cannot get post prior in post-thread tree!");
				node_pointer tmp;
				if(node->left_is_thread){
					tmp=node->left_child;
				}else if(node->right_child){
					tmp=node->right_child;
				}else{
					tmp=node->left_child;
				}
				return tmp;
			}

			
		public:
			t_tree(t_tree && tree):b_tree<DataType,NodeType>((b_tree<DataType,NodeType>&&)tree) {
				_thread_nodes=std::move(tree._thread_nodes);
				_kind=tree._kind;
			}
			~t_tree(){
				un_thread();
			}
			// return a shared tree holding the same nodes.
			[[nodiscard("Unused Tree")]] t_tree make_shared() {
				t_tree<DataType,NodeType> Ret(basic_type::_root);
				Ret._owned=false;
				Ret._thread_nodes=std::move(_thread_nodes);
				Ret._kind=_kind;
				Ret.num_of_nodes=basic_type::num_of_nodes;
				return Ret;
			}
			t_tree():t_tree(nullptr){}
			t_tree(const t_tree&)=delete;
			std::string to_string() const override {
				std::string s;
				s.append("<-Thread tree ");
				if(_kind==THREAD_KIND::UNTHREADED){
					s.append("(No thread)");
				}else if(_kind==THREAD_KIND::THREAD_PRE){
					s.append("in pre-order");
				}else if(_kind==THREAD_KIND::THREAD_IN){
					s.append("in in-order");
				}else{
					s.append("in post-order");
				}
				s.append("->");
				return s;
			}
			// handle the last node.
			void tree_in_thread() {
				assert(_kind==THREAD_KIND::UNTHREADED&&"Already thread!");
				node_pointer node=nullptr;
				tree_in_thread_impl(basic_type::_root,node);
				if(!node->right_child){
					node->right_is_thread=true;
					_thread_nodes.insert(node);
				}
				_kind=THREAD_KIND ::THREAD_IN;
			}
			void tree_pre_thread(){
				assert(_kind==THREAD_KIND::UNTHREADED&&"Already thread!");
				node_pointer node=nullptr;
				tree_pre_thread_impl(basic_type::_root,node);
				if(!node->right_child){
					node->right_is_thread=true;
					_thread_nodes.insert(node);
				}
				_kind=THREAD_KIND ::THREAD_PRE;
			}
			void tree_post_thread(){
				assert(_kind==THREAD_KIND::UNTHREADED&&"Already thread!");
				node_pointer node=nullptr;
				tree_post_thread_impl(basic_type::_root,node);
				if(!node->right_child){
					node->right_is_thread=true;
					_thread_nodes.insert(node);
				}
				_kind=THREAD_KIND ::THREAD_POST;
			}


			void un_thread(){
				for(auto& leaf:_thread_nodes){
					if(leaf->left_is_thread){
						leaf->left_is_thread=false;
						leaf->left_child=nullptr;
					}
					if(leaf->right_is_thread){
						leaf->right_is_thread=false;
						leaf->right_child=nullptr;
					}
				}
				_thread_nodes.clear();
				_kind=THREAD_KIND::UNTHREADED;
			}

			bool is_pre_thread()const{
				return _kind==THREAD_KIND::THREAD_PRE;
			}
			bool is_in_thread()const{
				return _kind==THREAD_KIND::THREAD_IN;
			}
			bool is_post_thread()const{
				return _kind==THREAD_KIND::THREAD_POST;
			}


			template <bool Echo = true>
			void in_order_with_thread(node_pointer start,std::ostream &out)const {
				assert(_kind==THREAD_KIND::THREAD_IN&&"Cannot in order tranverse in non-in-thread tree!");
				node_pointer tmp;
				for(tmp=in_first(start);tmp;tmp=in_after(tmp)){
					if (Echo) {
						print_visiting_node(tmp,out);
						out<<'\n';
					}
				}
			}
			template <bool Echo = true>
			void pre_order_with_thread(node_pointer start,std::ostream &out) const {
				assert(_kind==THREAD_KIND::THREAD_PRE&&"Cannot pre order tranverse in non-pre-thread tree!");
				node_pointer tmp;
				for(tmp=pre_first(start);tmp;tmp=pre_after(tmp)){
					if (Echo){
						print_visiting_node(tmp,out);
						out<<'\n';
					}
				}
			}
			template <bool Echo = true,bool ShowNullNode=false>
			void post_order(node_pointer start,std::ostream &out)const {
				/*we can not leverage post order threads*/
				std::cerr<<"We cannot leverage post order threads.\n";
			}
			
			[[nodiscard("Unused Tree")]] static t_tree create_empty_tree() {
				t_tree Ret(nullptr);
				return Ret;
			}
			// levelly input.
			[[nodiscard("Unused Tree")]] static t_tree create_tree_l(std::istream &in=std::cin){
				t_tree Ret(nullptr);
				char Indicator;
				size_t size=0;
				DataType Data;
				node_pointer pop;
				std::queue<node_pointer> tmp_queue;
				// !!! allowed to read a char of indicator.
				root_input:Indicator=in.peek();
				if(Indicator=='\n'){
					in.get();
					goto root_input;
				}else if (Indicator==EMPTY_NODE_INDICATOR) {
					in.get();
					Ret._root=nullptr;
					return Ret;
				}else if(Indicator==' '||Indicator=='\t'){
					in.get();
					// read next input.
					return Ret;
				}else {
					// take back to stream.
					in>> Data;
					node_pointer node=new node_type(Data);
					Ret._root=node;
					++basic_type::num_of_nodes;
					tmp_queue.push(Ret._root);
				}
				while (!tmp_queue.empty()){
					pop = tmp_queue.front();
					for(auto It=pop->child_begin();It!=pop->child_end();++It){
						// !!! allowed to read a char of indicator.
						re_input:Indicator=in.peek();
						if(Indicator=='\n'){
							in.get();
							goto re_input;
						}else if (Indicator==EMPTY_NODE_INDICATOR) {
							*It=nullptr;
							in.get();
							continue;
						}else if(Indicator==' '||Indicator=='\t'){
							in.get();
							// read next input.
							continue;
						}else {
							in>> Data;
							++basic_type::num_of_nodes;
							node_pointer node=new node_type(Data);
							*It=node;
							pop->is_leaf=false;
							node->parent=pop;
							++pop->child_size;
							tmp_queue.push(node);
							
						}	
					}
					Ret.shift_height(pop);
					tmp_queue.pop();
				}
				return Ret;
			}
			// recursively input.
			[[nodiscard("Unused Tree")]] static t_tree<DataType,NodeType> create_tree_r(std::istream &in=std::cin){
				t_tree<DataType,NodeType> Ret(nullptr);
				Ret._root=Ret.create_node_impl(in);
				return Ret;
			}
			void print_visiting_node(node_pointer node,std::ostream &out)const{
				basic_type::print_visiting_node(node,out);
				if(node->left_is_thread){
					out<<" ,left is thread";
				}
				if(node->right_is_thread){
					out<<" ,right is thread";
				}
			}
		};
	}
}

#endif
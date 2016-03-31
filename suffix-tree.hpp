#include <iostream>
#include <string>      
#include <cstdlib>
#include "optimize.h"

namespace suffix_tree
{
	struct memory_pool
	{
		char *begin_ptr;
		char *cu_ptr;
		memory_pool()
		{
			reserve(1 << 16);
		}
		void reserve(size_t n_bytes)
		{
			begin_ptr = (char*)malloc(n_bytes);
			cu_ptr    = begin_ptr;
		}
		template <typename T>
			T* alloc(int count = 1)
			{
				T* rez = (T*)cu_ptr;
				cu_ptr += sizeof(T) * count;
				return rez;
			}
		~memory_pool()
		{
			free((void*)begin_ptr);
		}
	};
  template<typename CharT, typename CharIterator=const CharT*>
  struct memory_model_base
  {
    typedef CharT char_t;
    typedef CharIterator iterator_t;
#if 0
    type node_t;
    struct transition_t
    {
      iterator_t begin();
      iterator_t begin(iterator_t new_begin);
      iterator_t end();
      iterator_t end(iterator_t new_end);
      node_t to();
      node_t to(node_t new_to);
    };

    iterator_t infinity();
    node_t bottom();
    node_t root();

    node_t new_leaf();
    node_t new_internal_node();
    
    bool transition(transition_t& rez, node_t &s, iterator_t p_chr);
    node_t parent(node_t s);
    node_t parent(node_t s, node_t new_parent);
    bool is_leaf(node_t s);
    
#endif
  };

  template<typename memory_model>
  struct suffix_tree_algorithm : memory_model
  {
    typedef memory_model base;
    typedef typename base::char_t char_t;
    typedef typename base::iterator_t iterator_t;
    typedef typename base::transition_t transition_t;
    typedef typename base::node_t node_t;


    node_t bottom()
    {
      return base::bottom();
    }
    node_t root()
    {
      return base::root();
    }
    node_t root(node_t new_root)
    {
      return base::root(new_root);
    }

    node_t new_leaf()
    {
      return base::new_leaf();
    }
    node_t new_internal_node()
    {
      return base::new_internal_node();
    }
    
    bool transition(transition_t& rez, node_t &s, iterator_t c)
    {
      return base::transition(rez, s, c);
    }
    node_t parent(node_t s)
    {
      return base::parent(s);
    }
    node_t parent(node_t s, node_t new_parent)
    {
      return base::parent(s, new_parent);
    }
    bool is_leaf(node_t s)
    {
      return base::is_leaf(s);
    }
    iterator_t infinity()
    {
      return base::infinity();
    }

    void update(node_t &s, iterator_t& k, iterator_t i)
    {
      node_t oldr = root();
      node_t r;
      while(!test_and_split(r, s, k, i - 1, i - 1)) {
        transition_t t;
        node_t rp = new_leaf();
        transition(t, r, i - 1);
        t.begin(i - 1);
        t.end(infinity());
        t.to(rp);
        if(oldr != root()) {
          parent(oldr, r);
        }
        oldr = r;
        s = parent(s);
        canonize(s, k, i - 1);
      }
      if(oldr != root()) {
        parent(oldr, r);
      }
    }
    bool test_and_split(node_t &rez, node_t &s, iterator_t &k, iterator_t p, iterator_t p_chr)
    {
      if (k < p) {
        transition_t tk;
        transition(tk, s, k);
        iterator_t new_end = tk.begin() + (p - k);
        if (*p_chr == *new_end) {
          rez = s;
          return true;
        } else {
          node_t r = new_internal_node();
          transition_t tr;
          transition(tr, r, new_end);
          tr.begin(new_end);
          tr.end(tk.end());
          tr.to(tk.to());
          tk.end(new_end);
          tk.to(r);
          rez = r;
          return false;
        }
      } else {
        transition_t tk;
        rez = s;
        return transition(tk, s, p_chr);
      }
    }
    void canonize(node_t &s, iterator_t &k, iterator_t p)
    {
      if (p <= k) {
        return;
      }
      transition_t tk;
      if (transition(tk, s, k)) {
        while (tk.end() - tk.begin() <= p - k) {
          k += (tk.end() - tk.begin());
          s = tk.to();
          if (k < p) {
            transition(tk, s, k);
          }
        }
      }
    }
  };
  template<int N, typename CharT, typename CharIterator, typename MemoryPool>
  struct limited_memory_model : memory_model_base<CharT, CharIterator>, MemoryPool
  {
    struct node_class;
    typedef node_class *node_t;
    typedef memory_model_base<CharT, CharIterator> base;
    typedef typename base::char_t char_t;
    typedef typename base::iterator_t iterator_t;
    iterator_t begin, end;
    node_t m_root;
    size_t n_nodes;
    struct node_class
    {
      bool is_leaf;
      typedef unsigned long pntr_int;
      void print()
      {
	      std::cout << "n" << (pntr_int)this << " [label=\"\",shape=point]" << std::endl;
      }
    };    
    limited_memory_model()
    {
	    n_nodes = 0;
    }
    size_t get_num_nodes()
    {
	    return n_nodes;
    }
    node_t bottom()
    {
      return 0;
    }
    node_t root()
    {
      return m_root;
    }
    iterator_t infinity()
    {
      return end;
    }
    struct leaf_node : node_class
    {
    }; 
    struct transition_data
    {
      iterator_t begin, end; 
      node_t to;
    };
    struct internal_node : node_class
    {                 
	    typedef typename node_class::pntr_int pntr_int;
	    static void print(node_t n)
	    {
		    n->print();
		    if (!n->is_leaf)
			    ((internal_node*)n)->print();
	    }
      	    void print()
	    {
		    std::cout << "n" << (pntr_int)this << " [label=\"\"]" << std::endl;
		    for (int i = 0; i < N; ++i) {
			    if (children[i].to != 0) {
				    print(children[i].to);
				    std::cout << "n" << (pntr_int)this << " -> n" << (pntr_int) children[i].to << " [label=\"";
				    iterator_t begin = children[i].begin;
				    iterator_t end   = children[i].end;
				    for(;begin < end; ++begin)
					    std::cout << (char)(*begin + ('a' - 1));
				    std::cout <<"\"]" << std::endl;
			    } else {
//				    std::cerr << "zero child" << std::endl;
			    }
		    }
		    std::cout << "n" << (pntr_int)this << " -> n" << (pntr_int)parent << " [style=dashed, constraint=false]" << std::endl;
      	    }
	    transition_data children[N];
	    node_t parent;
    };
    node_t new_leaf()
    {
      	    node_t rez = MemoryPool::template alloc<leaf_node>();
	    rez->is_leaf = true;
	    n_nodes++;
	    return rez;
    }
    node_t new_internal_node()
    {
       	    internal_node *rez = MemoryPool::template alloc<internal_node>();
	    rez->is_leaf = false;
	    rez->parent = rez;
	    for (int i = 0; i < N; ++i)
		    rez->children[i].to = 0;
	    n_nodes++;
	    return rez;
    }
    
    struct transition_t
    {
      transition_data *p_data;
      iterator_t      iter;
      node_t root;

      iterator_t begin()
      {
        if (likely(p_data))
          return p_data->begin;
        else
          return iter;
      }
      iterator_t begin(iterator_t new_begin)
      {
        if (unlikely(!p_data))
          THROW("modifying bottom");
        iterator_t old = p_data->begin;
        p_data->begin = new_begin;
        return old;
      }
      iterator_t end()
      {
        if (likely(p_data))
          return p_data->end;
        else 
          return iter + 1;
      }
      iterator_t end(iterator_t new_end)
      {
        if (unlikely(!p_data))
          THROW("modifying bottom");
        iterator_t old = p_data->end;
        p_data->end = new_end;
        return old;
      }

      node_t to()
      {
        if (likely(p_data)) {
		return p_data->to;
	}
        else
          return root;
      }
      node_t to(node_t new_to)
      {
        if (unlikely(!p_data))
          THROW("modifying bottom");
        node_t old = p_data->to;
        p_data->to = new_to;

        return old;
      }
    };

    bool transition(transition_t& rez, node_t &s, iterator_t p_chr)
    {
      if (s == bottom()) {
        rez.p_data = 0;
        rez.iter = p_chr;
        rez.root = m_root;
        return true;
      } else {
        if (unlikely(is_leaf(s))) {
		THROW("getting a leaf's transitions");
        } else {
          internal_node *n = (internal_node*)s;
          rez.p_data = n->children + *p_chr;
	  rez.iter = p_chr;
	  rez.root = 0;
          return rez.p_data->to != 0;
        }
      }
    }
    node_t parent(node_t s)
    {
      if (unlikely(is_leaf(s))) {
        THROW("getting a leaf's parent");
      } else {
        internal_node *n = (internal_node*)s;
        return n->parent;
      }
    }
    node_t parent(node_t s, node_t new_parent)
    {
      if (unlikely(is_leaf(s))) {
        THROW("setting a leaf's parent");
      } else {
        internal_node *n = (internal_node*)s;
        node_t old = n->parent;
        n->parent = new_parent;
        return old;
      }
    }
    static bool is_leaf(node_t s)
    {
      return s->is_leaf;
    }
  };
  template <int N, typename CharT, typename CharIterator = const char*, typename MemoryPool = memory_pool>
  struct suffix_tree : suffix_tree_algorithm<limited_memory_model<N, CharT, CharIterator, MemoryPool> >
  {
    typedef suffix_tree_algorithm<limited_memory_model<N, CharT, CharIterator, MemoryPool> > base;
    typedef typename base::char_t char_t;
    typedef typename base::iterator_t iterator_t;
    typedef typename base::node_t node_t;
    suffix_tree()
    {
      this->m_root = base::new_internal_node();
      base::parent(this->m_root, base::bottom());
    }
    void add_word(iterator_t begin, iterator_t end)
    {
      node_t s = this->m_root;
      this->begin = begin;
      this->end   = end;
      
      iterator_t k = begin;
      for (; begin < end; ++begin)
      {
        base::update(s, k, begin + 1);
        base::canonize(s, k, begin + 1);
      }
    }
    void print()
    {
	    base::internal_node::print(this->m_root);
    }
  };
}

template <int Nchars, typename CharT = char, CharT def=255>
struct remapper
{
	CharT mapped[Nchars];
	CharT next_code;
	CharT max_code;
	remapper(CharT first_code, CharT max_code)
	{
		next_code = first_code;
		this->max_code = max_code;
		for (int i = 0; i < Nchars; ++i)
			mapped[i] = def;
	}
	void map(CharT what, CharT to)
	{
		mapped[(long)what] = to;
		if (next_code == to) next_code++;
	}
	CharT remap(CharT c)
	{
		if (unlikely(mapped[(long)c] == def)) {
			if (max_code == next_code)
				THROW("remapper error");
			mapped[(long)c] = next_code++;
		}
		return mapped[(long)c];
	}
	template <typename CharIterator>
		void remap_range(CharIterator begin, CharIterator end)
		{
			while (begin < end) {
				*begin = remap(*begin);
				begin++;
			}
		}
};
#if 0
int main()
{
  try {
		remapper<256> rmp(1);
		rmp.map(' ', 0);
		char chr;
		std::string str;
		std::getline(std::cin, str);
    suffix_tree::suffix_tree<6, char, std::string::const_iterator> st(1<<16);
		std::string::iterator prev=str.begin(), cu;
		for (cu = prev; cu < str.end(); ++cu) {
			*cu = rmp.remap(*cu);
			std::cerr << "char = " << (int)*cu << std::endl;
			if (*cu == 0) {
				st.add_word(prev, cu + 1);
				std::cerr << "added " << prev - str.begin() << " " << cu + 1  - str.begin() << std::endl;
				prev = cu + 1;
			}
		}
    std::cout << "digraph ST {" << std::endl;
    st.print();
    std::cout << "}" << std::endl;
  } catch(const char *str) {
    std::cout << "exception: " << str << std::endl;
  }

  return 0;
}
#endif

/**
 * @file trie.hpp
 * @author JDongChen
 * @brief
 * @version 0.1
 * @date 2022-08-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SNOWY_TRIE_H
#define SNOWY_TRIE_H

#include <cassert>

#include <functional>
#include <map>

#include <string>

// forward declaration
template <typename K, typename T, class Compare = std::less<K>>
class radix_tree;
template <typename K, typename T, class Compare = std::less<K>>
class radix_tree_node;

// -------------------radix_tree_iter-----------------------------//

template <typename K, typename T, class Compare = std::less<K>>
class radix_tree_iter
    : public std::iterator<std::forward_iterator_tag, std::pair<K, T>> {
  friend class radix_tree<K, T, Compare>;

private:
  radix_tree_node<K, T, Compare> *pointee_;
  radix_tree_iter(radix_tree_node<K, T, Compare> *p) : pointee_(p) {}
  /**
   * @brief 寻找到leaf节点
   *
   * @param node
   * @return radix_tree_node<K, T, Compare>*
   */
  radix_tree_node<K, T, Compare> *
  descend(radix_tree_node<K, T, Compare> *node) const {
    if (node->is_leaf_) {
      return node;
    }
    typename radix_tree_node<K, T, Compare>::it_child it =
        node->children_.begin();

    assert(it != node->children_.end());
    return descend(it->second);
  }
  /**
   * @brief
   *
   * @param node
   * @return radix_tree_node<K, T, Compare>*
   */
  radix_tree_node<K, T, Compare> *
  increment(radix_tree_node<K, T, Compare> *node) const {
    radix_tree_node<K, T, Compare> *parent = node->parent_;
    if (parent == nullptr)
      return nullptr;
    typename radix_tree_node<K, T, Compare>::it_child it =
        parent->children_.find(node->key_);
    assert(it != parent->children_.end());
    ++it; // 下一个key_

    if (it == parent->children_.end())
      return increment(parent);
    else
      return descend(it->second);
  }

public:
  radix_tree_iter() : pointee_(nullptr) {}
  radix_tree_iter(const radix_tree_iter &other) : pointee_(other.pointee_) {}
  radix_tree_iter &operator=(const radix_tree_iter &other) {
    pointee_ = other.pointee_;
    return *this;
  }
  std::pair<const K, T> &operator*() const { return *pointee_->value_; }
  std::pair<const K, T> &operator->() const { return pointee_->value_; }
  const radix_tree_iter<K, T, Compare> &operator++() {
    if (pointee_ != nullptr) {
      pointee_ = increment(pointee_);
    }
    return *this;
  }
  const radix_tree_iter<K, T, Compare> operator++(int) {
    radix_tree_iter<K, T, Compare> copy(*this);
    ++(*this);
    return copy;
  }
  bool operator!=(const radix_tree_iter<K, T, Compare> &other) const {
    return pointee_ != other.pointee_;
  }
  bool operator==(const radix_tree_iter<K, T, Compare> &other) const {
    return pointee_ == other.pointee_;
  }
};

//======================radix_tree_node=======================================
template <typename K, typename T, typename Compare>

class radix_tree_node {
  friend class radix_tree<K, T, Compare>;
  friend class radix_tree_it<K, T, Compare>;

public:
  using key_type = K;
  using mapped_type = T;
  using value_type = std::pair<const K, T>;
  using iter_child =
      std::map<K, radix_tree_node<K, T, Compare> *, Compare>::iterator;

private:
  /**
   * @brief
   * 父节点：node
   * 子节点：key和node组成的 map
   * 深度：depth
   * 叶子节点：只有叶子节点代表字符串的存在
   * 键：key_
   * 值：value_
   */

  int depth_;
  bool is_leaf_;

  key_type key_;
  value_type *value_;
  Compare &pred_;

  radix_tree_node<K, T, Compare> *parent_;
  std::map<K, radix_tree_node<K, T, Compare> *, Compare> children_;

private:
  radix_tree_node(Compare &pred)
      : children_(std::map<K, radix_tree_node<K, T, Compare> *, Compare>(pred)),
        parent_(nullptr), value_(nullptr), depth_(0), is_leaf_(false), key_(),
        pred_(pred) {}
  radix_tree_node(const value_type &val, Compare &pred)
      : children_(std::map<K, radix_tree_node<K, T, Compare> *, Compare>(pred)),
        parent_(nullptr), depth_(0), is_leaf_(false), key_(), pred_(pred) {
    value_ = new value_type(val);
  }
  radix_tree_node(const radix_tree_node &) = delete;            // delete
  radix_tree_node &operator=(const radix_tree_node &) = delete; // delete
  ~radix_tree_node() {
    iter_child it;
    for (it = children_.begin(); it != children_.end(); ++it) {
      delete it->second;
    }
    delete value_;
  }
};

//========================radix_tree===========================//
template <typename K, typename T, typename Compare>

class radix_tree {

public:
  using key_type = K;
  using mapped_type = T;
  using value_type = std::pair<const K, T>;
  using size_type = std::size_t;
  using iterator = radix_tree_iter<K, T, Compare>;

private:
  size_type size_;
  radix_tree_node<K, T, Compare> *root_;
  Compare predicate_;

public:
  radix_tree() : size_(0), root_(nullptr), predicate_(Compare()) {}
  explicit radix_tree(Compare pred)
      : size_(0), root_(nullptr), predicate_(pred) {}
  ~radix_tree() { delete root_; }
  radix_tree(const radix_tree &other) = delete;           // delete
  radix_tree &operator=(const radix_tree other) = delete; // delete
  size_type size() const { return size_; }
  bool empty() const { return size_ == 0; }
  void clear() {
    delete root_;
    root_ = nullptr;
    size_ = 0;
  }

  iterator begin() {
    radix_tree_node<K, T, Compare> *node;
    if (root_ = nullptr || size_ == 0)
      node = nullptr;
    else
      node = begin(root_);
    return iterator(node);
  }
  iterator end() { return iterator(nullptr); }
  T &operator[](const K &key) {
    iterator it = find(key);
    if (it == end()) {
      std::pair<K, T> val;
      val.first = key;
      std::pair<iterator, bool> ret;
      ret = insert(val);
      assert(ret.second == true);
      it = ret.first;
    }
    return it->second;
  }

public:
  std::pair<iterator, bool> insert(const value_type &val);
  iterator find(const K &key);
  bool erase(const K &key);
  bool erase(iterator iter);

public:
  void prefix_match(const K &key, std::vector<iterator> &vec);
  void greedy_match(const K &key, std::vector<iterator> &vec);
  iterator longest_match(const K &key);
  template <class _UnaryPred> void remove_if(_UnaryPred pred) {
    radix_tree<K, T, Compare>::iterator backIt;
    for (radix_tree<K, T, Compare>::iterator it = begin(); it != end();
         it = backIt) {
      backIt = it;
      backIt++;
      K toDelete = (*it).first;
      if (pred(toDelete)) {
        erase(toDelete);
      }
    }
  }

public:
  iterator longest_match(const K &key);

private:
  radix_tree_node<K, T, Compare> *begin(radix_tree_node<K, T, Compare> *node) {
    if (node->is_leaf_)
      return node;
    assert(!node->children_.empty());
    return begin(node->children_.begin()->second);
  }

  radix_tree_node<K, T, Compare> *
  find_node(const K &key, radix_tree_node<K, T, Compare> *node, int depth);
  radix_tree_node<K, T, Compare> *append(radix_tree_node<K, T, Compare> *parent,
                                         const value_type &val);
  radix_tree_node<K, T, Compare> *prepend(radix_tree_node<K, T, Compare> *node,
                                          const value_type &val);
  void greedy_match(radix_tree_node<K, T, Compare> *node,
                    std::vector<iterator> &vec);
};

// type traits for string
template <typename K> K radix_substr(const K &key, int begin, int num);
template <>
inline std::string radix_substr<std::string>(const std::string &key, int begin,
                                             int num) {
  return key.substr(begin, num);
}

template <typename K> K radix_join(const K &key1, const K &key2);
template <>
inline std::string radix_join<std::string>(const std::string &key1,
                                           const std::string &key2) {
  return key1 + key2;
}

template <typename K> int radix_length(const K &key);
template <> inline int radix_length<std::string>(const std::string &key) {
  return static_cast<int>(key.size());
}

template <typename K, typename T, typename Compare>
void radix_tree<K, T, Compare>::prefix_match(const K &key,
                                             std::vector<iterator> &vec) {
  vec.clear();
  if (root_ == nullptr)
    return; //
  radix_tree_node<K, T, Compare> *node;
  K key_sub_1, key_sub_2;
  node = find_node(key, root_, 0);
  if (node->is_leaf_) {
    node = node->parent_;
  }
  int len = radix_length(key) - node->depth_;
  key_sub_1 = radix_substr(key, node->depth_, len);
  key_sub_2 = radix_substr(node->key_, 0, len);
  if (key_sub_1 != key_sub_2)
    return;
  greedy_match(node, vec);
}

template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare> *radix_tree<K, T, Compare>::find_node(
    const K &key, radix_tree_node<K, T, Compare> *node, int depth) {
  if (node->children_.empty())
    return node;
  typename radix_tree_node<K, T, Compare>::it_child it;
  int len_key = radix_length(key) - depth; //
  for (it = node->children_.begin(); it != node->children_.end(); ++it) {
    if (len_key == 0) {
      if (it->second->is_leaf_) {
        return it->second;
      } else {
        continue;
      }
    }
    if (!it->second->is_leaf_ && key[depth] == it->first[0]) {
      int len_node = radix_length(it->first);
      K key_sub = radix_substr(key, depth, len_node);
      if (key_sub == it->first) {
        return find_node(key, it->second, depth + len_node);
      } else {
        return it->second;
      }
    }
  }
  return node;
}

//----------------------------------------------------------------//

#endif /* SNOWY_TRIE_H */
/*
 * tree_node.h - A tree container using std::vector
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef AIQ_CORE_TREE_NODE_H
#define AIQ_CORE_TREE_NODE_H

#include <algorithm>
#include <vector>

namespace RkCam {

template <typename T>
class TreeNode {
 public:
    using value_type = T;

    TreeNode() : parent_(nullptr){};
    TreeNode(const T& t, TreeNode* parent = nullptr) {
        this->value_  = t;
        this->parent_ = parent;
    }
    TreeNode(const TreeNode& other) {
        this->value_    = other.value_;
        this->parent_   = other.parent_;
        this->children_ = other.children_;
        std::for_each(this->children_.begin(), this->children_.end(),
                      [this](const T& t) { t.parent_ = this; });
    }
    const TreeNode& operator=(const TreeNode& other) {
        if (this != &other) {
            this->value_    = other.value_;
            this->parent_   = other.parent_;
            this->children_ = other.children_;
            std::for_each(this->children_.begin(), this->children_.end(),
                          [this](const T& t) { t.parent_ = this; });
        }
        return *this;
    }
    virtual ~TreeNode() = default;

    void AddChild(const T& t) { children_.push_back(TreeNode(t, this)); }

    void RemoveChild(const T& t) {
        children_.erase(std::remove_if(children_.begin(), children_.end(),
                                       [&t](const T& other) { return other == t; }),
                        children_.end());
    }

    void SetValue(const T& t) { value_ = t; }

    T& GetValue() { return value_; }

    const T& GetValue() const { return value_; }

    std::vector<TreeNode>& GetChildren() { return children_; }

    const std::vector<TreeNode>& GetChildren() const { return children_; }

 private:
    T value_;
    T* parent_;
    std::vector<TreeNode> children_;
};

}  // namespace RkCam

#endif  // AIQ_CORE_TREE_NODE_H

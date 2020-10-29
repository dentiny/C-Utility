#ifndef SKIPLIST_HPP__
#define SKIPLIST_HPP__

class Skiplist 
{
private:
    struct Node
    {
        int val;
        Node * right; // every singly-linked list is arranged left-right
        Node * down; // singly-linked lists are arranged up-down
        Node(Node * r, Node * d, int v) : 
            val { v },
            right { r },
            down { d }
            {}
    };

public:
    Skiplist() :
        head { new Node(nullptr, nullptr, 0) }
        {}
    
    // (1) first move right as possible
    // (2) then move down as possible
    bool search(int val) 
    {
        Node * cur = head;
        while(cur != nullptr)
        {
            // search in the highest level(with minimum nodes)
            while(cur->right != nullptr && cur->right->val < val) cur = cur->right;
            
            // move down when possible
            if(cur->right == nullptr || cur->right->val > val) cur = cur->down; 
            
            // cur->right != nullptr && cur->right->val <= val
            // but if cur->right->val < val, then it will be trapped in while loop
            // so cur->right->val == val, thus find the val
            else return true;
        }
        return false;
    }
    
    void add(int val) 
    {
        stack<Node*> insertPoints;
        Node * cur = head;
        while(cur != nullptr)
        {
            // search from the highest level(with minimum number)
            // move right until 
            // (1) cur->val < val and cur->right is nullptr or 
            // (2) cur->val < val and cur->right->val >= val
            while(cur->right != nullptr && cur->right->val < val) cur = cur->right;
            
            insertPoints.push(cur);
            cur = cur->down;
        }
        
        Node * downNode = nullptr;
        bool insertUp = true;
        while(insertUp && !insertPoints.empty())
        {
            cur = insertPoints.top(); insertPoints.pop();
            cur->right = new Node(cur->right, downNode, val);
            downNode = cur->right;
            insertUp = (rand() & 1) == 0;
        }
        
        // current head node as down-node
        // create newly-emerged node, and as right node of the head node 
        if(insertUp) head = new Node(new Node(nullptr, downNode, val), head, 0);
    }
    
    // search the val first, then skip it in the singly-linked list
    // move downwards for further deletion
    bool erase(int val) 
    {
        Node * cur = head;
        bool seen = false;
        while(cur != nullptr)
        {
            // search in the highest level(with minimum nodes)
            while(cur->right != nullptr && cur->right->val < val) cur = cur->right;
            
            // move down when possible
            if(cur->right == nullptr || cur->right->val > val) cur = cur->down;
            
            // same as search(), has found the val
            else
            {
                // delete the right node, then move down
                seen = true;
                Node * temp = cur->right;
                cur->right = temp->right;
                delete temp;
                cur = cur->down;
            }
        }
        return seen;
    }
    
private:
    Node * head;
};

#endif
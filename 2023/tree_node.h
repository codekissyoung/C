#ifndef TREE_NODE_H
#define TREE_NODE_H
// 定义二叉树节点结构体
typedef struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

void preorderTraversal(TreeNode* root);
void inorderTraversal(TreeNode* root);
void postorderTraversal(TreeNode* root);
TreeNode* createNode(int value);

#endif

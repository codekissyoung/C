#include <stdio.h>
#include <stdlib.h>
#include "tree_node.h"

void treeDemo()
{
    // 构建一个简单的二叉树
    TreeNode* root = createNode(1);
    root->left = createNode(2);
    root->right = createNode(3);
    root->left->left = createNode(4);
    root->left->right = createNode(5);
    root->right->left = createNode(6);
    root->right->right = createNode(7);

    printf("前序遍历: ");
    preOrderTraversal(root);
    printf("\n");

    printf("中序遍历: ");
    inOrderTraversal(root);
    printf("\n");

    printf("后序遍历: ");
    postOrderTraversal(root);
    printf("\n");
}

// 前序遍历
void preOrderTraversal(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    printf("%d ", root->val);
    preOrderTraversal(root->left);
    preOrderTraversal(root->right);
}

// 中序遍历
void inOrderTraversal(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    inOrderTraversal(root->left);
    printf("%d ", root->val);
    inOrderTraversal(root->right);
}

// 后序遍历
void postOrderTraversal(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    postOrderTraversal(root->left);
    postOrderTraversal(root->right);
    printf("%d ", root->val);
}

// 创建节点的辅助函数
TreeNode* createNode(int value) {
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    newNode->val = value;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}
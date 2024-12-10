#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <vector>
#include <string>
#include <map>
#include <chrono>
using namespace std;

// 二叉搜索树节点
struct BSTNode {
    string key;
    string value;
    BSTNode* left;
    BSTNode* right;
    BSTNode(const string& k, const string& v) : key(k), value(v), left(nullptr), right(nullptr) {}
};


// AVL树节点
struct AVLNode {
    string key;
    string value;
    AVLNode* left;
    AVLNode* right;
    int height; // 平衡因子

    AVLNode(const string& k, const string& v) : key(k), value(v), left(nullptr), right(nullptr), height(1) {}
};

// 红黑树节点
struct RBNode {
    string key;
    string value;
    RBNode* left;
    RBNode* right;
    RBNode* parent;
    bool isRed; // 红黑标志
    RBNode(const string& k, const string& v) : key(k), value(v), left(nullptr), right(nullptr), parent(nullptr), isRed(true) {}
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_lineEdit_textChanged(const QString& text);
    void on_buttonClicked();

private:
    QLineEdit* lineEdit;
    QListWidget* listWidget;
    QPushButton* searchButton;

    // 数据存储
   vector<pair<string, string>> m_allWords; // 顺序查找
   map<char, BSTNode*> bstMap; // 依照首字母建立二叉树
   map<char, AVLNode*> avlMap;  // AVL 树
   map<char, RBNode*> rbMap; // 红黑树

    void loadDictionary(const QString& fileName);
    vector<string> prefixSearchSequential(const string& prefix, int maxResults = 10); //按序查找
    vector<string> prefixSearchBST(BSTNode* root, const std::string& prefix, int maxResults = 10);
    vector<string> prefixSearchAVL(AVLNode* root, const string& prefix, int maxResults = 10);

    BSTNode* insertBST(BSTNode* root, const string& key, const string& value);
    AVLNode* insertAVL(AVLNode* root, const string& key, const string& value);
    RBNode* insertRB(RBNode* root, const string& key, const string& value);

    bool searchBST(BSTNode* root, const string& key, vector<string>& path, string& result);
    bool sequentialSearch(const string& key, vector<std::string>& path, std::string& result);
    bool searchAVL(AVLNode* root, const string& key, vector<string>& path, string& result);
     bool searchRB(RBNode* root, const string& key, vector<string>& path, string& result);

    void deleteBST(BSTNode* root);
    void deleteAllBSTs();
    void deleteAVL(AVLNode* root);
    void deleteAllAVLs();
    void deleteRB(RBNode* root);
    void deleteAllRBs();

    int compareKeys(const string& a, const string& b);
    // AVL 树的旋转
    AVLNode* rotateLeft(AVLNode* x);
    AVLNode* rotateRight(AVLNode* y);
    int getHeight(AVLNode* node);
    int getBalance(AVLNode* node);
    RBNode*fixInsertRB(RBNode* root, RBNode* node) ;
    RBNode*leftRotateRB(RBNode* root, RBNode* node);
    RBNode* rightRotateRB(RBNode* root, RBNode* node);
};

#endif

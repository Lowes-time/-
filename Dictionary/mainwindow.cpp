#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <algorithm>
#include <QTextEdit>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);

    // 输入框
    lineEdit = new QLineEdit(central);
    layout->addWidget(lineEdit);

    // 近似列表
    listWidget = new QListWidget(central);
    layout->addWidget(listWidget);

    // 搜索按钮
    searchButton = new QPushButton("查询中文翻译", central);
    layout->addWidget(searchButton);

    setCentralWidget(central);

    // 加载字典文件
    loadDictionary("E:/code qt/Dictionary/EnWords.csv");

    connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_textChanged);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::on_buttonClicked);
}

MainWindow::~MainWindow() {
    deleteAllBSTs();
    deleteAllAVLs();
}

void MainWindow::loadDictionary(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开字典文件！");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        line = line.mid(1, line.length() - 2);

        QStringList parts = line.split("\",\"");
        if (parts.size() == 1) {
            qWarning() << "信息缺失行:" << line;
           continue;
        }

        string word = parts[0].toStdString();
        string meaning = parts[1].toStdString();
        m_allWords.emplace_back(word, meaning);

        // 按首字母插入对应的树
        char firstChar = tolower(word[0]);
        if (bstMap.find(firstChar) == bstMap.end()) {
            bstMap[firstChar] = nullptr; // 初始化一个新的树
        }
        bstMap[firstChar] = insertBST(bstMap[firstChar], word, meaning);

        // 对AVL树进行插入
     if (avlMap.find(firstChar) == avlMap.end()) {
            avlMap[firstChar] = nullptr;
        }
        avlMap[firstChar] = insertAVL(avlMap[firstChar], word, meaning);

        // 插入 RB
      if (rbMap.find(firstChar) == rbMap.end()) {
            rbMap[firstChar] = nullptr;
        }
        rbMap[firstChar] = insertRB(rbMap[firstChar], word, meaning);
    }
    file.close();
    sort(m_allWords.begin(), m_allWords.end());
}


vector<string> MainWindow::prefixSearchSequential(const string& prefix, int maxResults) {
    vector<string> results;
    for (const auto& pair : m_allWords) {
        if (pair.first.find(prefix) == 0) {
            results.push_back(pair.first);
            if ((int)results.size() >= maxResults) break;//最多限制
        }
    }
    return results;
}

vector<string> MainWindow::prefixSearchBST(BSTNode* root, const string& prefix, int maxResults) {
    vector<string> results;
    function<void(BSTNode*)> dfs = [&](BSTNode* node) {
        if (!node || (int)results.size() >= maxResults) return;
        if (node->key.find(prefix) == 0) results.push_back(node->key);
        if (node->key >= prefix) dfs(node->left);
        dfs(node->right);
    };
    dfs(root);
    return results;
}



vector<string> MainWindow::prefixSearchAVL(AVLNode* root, const string& prefix, int maxResults) {
    vector<string> results;
    function<void(AVLNode*)> dfs = [&](AVLNode* node) {
        if (!node || (int)results.size() >= maxResults) return;
        if (node->key.find(prefix) == 0) results.push_back(node->key);
        if (node->key >= prefix) dfs(node->left);
        dfs(node->right);
    };
    dfs(root);
    return results;
}


//加箭头显示路径
string join(const vector<string>& vec, const string& delimiter) {
    string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i < vec.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}

BSTNode* MainWindow::insertBST(BSTNode* root, const string& key, const string& value) {
    if (!root) return new BSTNode(key, value);
    int comparison = compareKeys(key, root->key);
    if (comparison < 0) {
        root->left = insertBST(root->left, key, value);
    } else if (comparison > 0) {
        root->right = insertBST(root->right, key, value);
    }
    return root;
}

AVLNode* MainWindow::insertAVL(AVLNode* root, const string& key, const string& value) {
    if (!root) return new AVLNode(key, value);

    if (key < root->key) root->left = insertAVL(root->left, key, value);
    else if (key > root->key) root->right = insertAVL(root->right, key, value);
    else return root;

    root->height = 1 + max(getHeight(root->left), getHeight(root->right));
    int balance = getBalance(root);

    if (balance > 1 && key < root->left->key) return rotateRight(root);
    if (balance < -1 && key > root->right->key) return rotateLeft(root);
    if (balance > 1 && key > root->left->key) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }
    if (balance < -1 && key < root->right->key) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }
    return root;
}

AVLNode* MainWindow::rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;

    return y;
}

AVLNode* MainWindow::rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;

    return x;
}

int MainWindow::getHeight(AVLNode* node) {
    return node ? node->height : 0;
}

int MainWindow::getBalance(AVLNode* node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}
RBNode* MainWindow::insertRB(RBNode* root, const string& key, const string& value) {
    // 插入新的节点
    RBNode* newNode = new RBNode(key, value);
    if (root == nullptr) {
        newNode->isRed = false;  // 根节点总是黑色
        return newNode;
    }

    RBNode* parent = nullptr;
    RBNode* current = root;

    // 通过普通的二叉查找树插入新节点
    while (current != nullptr) {
        parent = current;
        if (key < current->key) {
            current = current->left;
        } else if (key > current->key) {
            current = current->right;
        } else {
            current->value = value;  // 如果关键字相等，更新值
            delete newNode;          // 释放新节点并返回当前根节点
            return root;
        }
    }

    // 设置父节点
    newNode->parent = parent;
    if (key < parent->key) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }

    // 2. 修正红黑树性质
    return fixInsertRB(root, newNode);
}

RBNode* MainWindow::fixInsertRB(RBNode* root, RBNode* node) {
    // 如果父节点是黑色或是根节点，直接返回
    while (node != root && node->parent->isRed == true) {
        if (node->parent == node->parent->parent->left) { // 父节点是左子树
            RBNode* uncle = node->parent->parent->right;
            if (uncle && uncle->isRed == true) { // Case 1: 叔叔是红色
                node->parent->isRed = false;      // 父节点变黑
                uncle->isRed = false;             // 叔叔变黑
                node->parent->parent->isRed = true; // 祖父节点变红
                node = node->parent->parent;        // 向上调整
            } else { // Case 2: 叔叔是黑色
                if (node == node->parent->right) { // Case 2a: 插入的是右子节点
                    node = node->parent;
                    root = leftRotateRB(root, node);  // 左旋
                }
                node->parent->isRed = false;       // 父节点变黑
                node->parent->parent->isRed = true; // 祖父节点变红
                root = rightRotateRB(root, node->parent->parent); // 右旋
            }
        } else { // 父节点是右子树
            RBNode* uncle = node->parent->parent->left;
            if (uncle && uncle->isRed == true) { // Case 1: 叔叔是红色
                node->parent->isRed = false;      // 父节点变黑
                uncle->isRed = false;             // 叔叔变黑
                node->parent->parent->isRed = true; // 祖父节点变红
                node = node->parent->parent;        // 向上调整
            } else { // Case 2: 叔叔是黑色
                if (node == node->parent->left) { // Case 2a: 插入的是左子节点
                    node = node->parent;
                    root = rightRotateRB(root, node); // 右旋
                }
                node->parent->isRed = false;       // 父节点变黑
                node->parent->parent->isRed = true; // 祖父节点变红
                root = leftRotateRB(root, node->parent->parent); // 左旋
            }
        }
    }
    root->isRed = false; // 根节点必须是黑色
    return root;
}

// 左旋操作
RBNode* MainWindow::leftRotateRB(RBNode* root, RBNode* node) {
    RBNode* rightChild = node->right;
    node->right = rightChild->left;
    if (rightChild->left != nullptr) {
        rightChild->left->parent = node;
    }
    rightChild->parent = node->parent;
    if (node->parent == nullptr) {
        root = rightChild; // 如果旋转的是根节点，更新根节点
    } else if (node == node->parent->left) {
        node->parent->left = rightChild;
    } else {
        node->parent->right = rightChild;
    }
    rightChild->left = node;
    node->parent = rightChild;

    return root;
}

// 右旋操作
RBNode* MainWindow::rightRotateRB(RBNode* root, RBNode* node) {
    RBNode* leftChild = node->left;
    node->left = leftChild->right;
    if (leftChild->right != nullptr) {
        leftChild->right->parent = node;
    }
    leftChild->parent = node->parent;
    if (node->parent == nullptr) {
        root = leftChild; // 如果旋转的是根节点，更新根节点
    } else if (node == node->parent->right) {
        node->parent->right = leftChild;
    } else {
        node->parent->left = leftChild;
    }
    leftChild->right = node;
    node->parent = leftChild;

    return root;
}


bool MainWindow::sequentialSearch(const string& key, vector<string>& path, string& result) {
    path.clear();
    for (const auto& wordPair : m_allWords) {
        path.push_back(wordPair.first);
        if (wordPair.first == key) {
            result = wordPair.second;
            return true;
        }
    }
    return false;
}

//search函数
bool MainWindow::searchBST(BSTNode* root, const string& key, vector<string>& path, string& result) {
    if (!root) return false;
    path.push_back(root->key);
    int comparison = compareKeys(key, root->key);
    if (comparison == 0) {
        result = root->value;
        return true;
    }
    if (comparison < 0) return searchBST(root->left, key, path, result);
    return searchBST(root->right, key, path, result);
}

bool MainWindow::searchAVL(AVLNode* root, const string& key, vector<string>& path, string& result) {
    if (!root) return false;
    path.push_back(root->key);
    int comparison = compareKeys(key, root->key);
    if (comparison == 0) {
        result = root->value;
        return true;
    }
    if (comparison < 0) return searchAVL(root->left, key, path, result);
    return searchAVL(root->right, key, path, result);
}

// 搜索 RB 函数实现
bool MainWindow::searchRB(RBNode* root, const string& key, vector<string>& path, string& result) {
    while (root) {
        path.push_back(root->key);
        int comparison = compareKeys(key, root->key);
        if (comparison == 0) {
            result = root->value;
            return true;
        } else if (comparison < 0) {
            root = root->left;
        } else {
            root = root->right;
        }
    }
    return false;
}

// 删除红黑树
void MainWindow::deleteRB(RBNode* root) {
    if (!root) return;
    deleteRB(root->left);
    deleteRB(root->right);
    delete root;
}

void MainWindow::deleteAllRBs() {
    for (auto& [key, root] : rbMap) {
        deleteRB(root);
    }
    rbMap.clear();
}

int MainWindow::compareKeys(const string& a, const string& b) {
    size_t minLength = min(a.length(), b.length());
    for (size_t i = 0; i < minLength; ++i) {
        if (a[i] < b[i]) return -1;
        if (a[i] > b[i]) return 1;
    }
    if (a.length() < b.length()) return -1;
    if (a.length() > b.length()) return 1;
    return 0;
}

void MainWindow::deleteBST(BSTNode* root) {
    if (!root) return;
    deleteBST(root->left);
    deleteBST(root->right);
    delete root;
}
void MainWindow::deleteAllBSTs() {
    for (auto& [key, root] : bstMap) {
        deleteBST(root);
    }
    bstMap.clear();
}

void MainWindow::deleteAVL(AVLNode* root) {
    if (!root) return;
    deleteAVL(root->left);
    deleteAVL(root->right);
    delete root;
}

void MainWindow::deleteAllAVLs() {
    for (auto& [key, root] : avlMap) {
        deleteAVL(root);
    }
    avlMap.clear();
}

void MainWindow::on_lineEdit_textChanged(const QString& text) {
    listWidget->clear();
    string prefix = text.toStdString();
    // if (!prefix.empty()) {
    //     auto candidates = prefixSearchSequential(prefix);
    //     for (const auto& word : candidates) {
    //         listWidget->addItem(QString::fromStdString(word));
    //     }
    // }
    if (!prefix.empty()) {
       char firstChar = tolower(prefix[0]);
      if (bstMap.find(firstChar) != bstMap.end()) {
           auto candidates = prefixSearchBST(bstMap[firstChar], prefix);
          for (const auto& word : candidates) {
             listWidget->addItem(QString::fromStdString(word));
         }
     }
   }
}
template<typename Func>
chrono::milliseconds measureExecutionTime(Func&& func) {
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start);
}

void MainWindow::on_buttonClicked() {
    QString input = lineEdit->text();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要查询的单词！");
        return;
    }

    string key = input.toStdString();
    char firstChar = tolower(key[0]);
    vector<string> path1,path2,path3,path4;
    string meaning1,meaning2,meaning3,meaning4;

    if (bstMap.find(firstChar) != bstMap.end() && searchBST(bstMap[firstChar], key, path1, meaning1)) {
        QMessageBox messageBox(nullptr);
        messageBox.setWindowTitle("查找方法");
        messageBox.setText("请选择查找方法");

        // 自定义按钮文本
        QPushButton* btnSequential = messageBox.addButton("顺序查找", QMessageBox::YesRole);
        QPushButton* btnBinaryTree = messageBox.addButton("二叉树查找", QMessageBox::NoRole);
        QPushButton* btnAVL = messageBox.addButton("AVL树查找", QMessageBox::YesRole);
        QPushButton* btnRB = messageBox.addButton("红黑树查找", QMessageBox::NoRole);
        // 显示对话框并等待用户选择
        messageBox.exec();
        chrono::milliseconds elapsedTime;
        if (messageBox.clickedButton() == btnBinaryTree) {
        elapsedTime = measureExecutionTime([&]() {
        searchBST(bstMap[firstChar], key, path1, meaning1);
        QString message = QString("路径：%1\n解释：%2")
                              .arg(QString::fromStdString(join(path1, " -> ")))
                              .arg(QString::fromStdString(meaning1));

        QWidget* messageWindow = new QWidget(nullptr);
        messageWindow->setWindowTitle("查询结果");
        QVBoxLayout* layout = new QVBoxLayout(messageWindow);
        //垂直布局
        QTextEdit* textEdit = new QTextEdit(messageWindow);
        textEdit->setText(message);
        textEdit->setReadOnly(true);
        layout->addWidget(textEdit);

        messageWindow->resize(400, 300);
        messageWindow->show();
        QMessageBox::information(this, "翻译为",
                                 QString::fromStdString(meaning1)); });

            // 显示时间
            QString timeMessage = QString("查询耗时: %1 毫秒").arg(elapsedTime.count());
            QMessageBox::information(this, "查询耗时", timeMessage);
        }

        else if (messageBox.clickedButton() == btnSequential) {
            elapsedTime = measureExecutionTime([&]() {
            sequentialSearch(key,path2,meaning2);
            // 显示路径的窗口
            QString pathMessage = QString("路径：%1").arg(QString::fromStdString(join(path2, " -> ")));
            QWidget* pathWindow = new QWidget(nullptr);
            pathWindow->setWindowTitle("顺序搜索路径");
            QVBoxLayout* pathLayout = new QVBoxLayout(pathWindow);
            QTextEdit* pathTextEdit = new QTextEdit(pathWindow);
            pathTextEdit->setText(pathMessage);
            pathTextEdit->setReadOnly(true);
            pathLayout->addWidget(pathTextEdit);
            pathWindow->resize(400, 300);
            pathWindow->show();

            QMessageBox::information(this, "翻译为",
                                     QString::fromStdString(meaning2));});

            // 显示时间
            QString timeMessage = QString("查询耗时: %1 毫秒").arg(elapsedTime.count());
            QMessageBox::information(this, "查询耗时", timeMessage);

        }
        else if (messageBox.clickedButton() == btnAVL) {
            elapsedTime = measureExecutionTime([&]() {
            searchAVL(avlMap[firstChar], key, path3, meaning3);
            QString message = QString("路径：%1\n解释：%2")
                                  .arg(QString::fromStdString(join(path3, " -> ")))
                                  .arg(QString::fromStdString(meaning3));

            QWidget* messageWindow = new QWidget(nullptr);
            messageWindow->setWindowTitle("查询结果");
            QVBoxLayout* layout = new QVBoxLayout(messageWindow);
            //垂直布局
            QTextEdit* textEdit = new QTextEdit(messageWindow);
            textEdit->setText(message);
            textEdit->setReadOnly(true);
            layout->addWidget(textEdit);

            messageWindow->resize(400, 300);
            messageWindow->show();
            QMessageBox::information(this, "翻译为",
                                     QString::fromStdString(meaning3));});

            // 显示时间
            QString timeMessage = QString("查询耗时: %1 毫秒").arg(elapsedTime.count());
            QMessageBox::information(this, "查询耗时", timeMessage);

        } else if (messageBox.clickedButton() == btnRB) {
            elapsedTime = measureExecutionTime([&]() {
            searchRB(rbMap[firstChar], key, path4, meaning4);
            QString message = QString("路径：%1\n解释：%2")
                                  .arg(QString::fromStdString(join(path4, " -> ")))
                                  .arg(QString::fromStdString(meaning4));

            QWidget* messageWindow = new QWidget(nullptr);
            messageWindow->setWindowTitle("查询结果");
            QVBoxLayout* layout = new QVBoxLayout(messageWindow);
            //垂直布局
            QTextEdit* textEdit = new QTextEdit(messageWindow);
            textEdit->setText(message);
            textEdit->setReadOnly(true);
            layout->addWidget(textEdit);

            messageWindow->resize(400, 300);
            messageWindow->show();
            QMessageBox::information(this, "翻译为",
                                     QString::fromStdString(meaning4));});

            // 显示时间
            QString timeMessage = QString("查询耗时: %1 毫秒").arg(elapsedTime.count());
            QMessageBox::information(this, "查询耗时", timeMessage);
        }
    } else {
        QMessageBox::warning(this, "未找到", "未找到该单词！");
    }
}

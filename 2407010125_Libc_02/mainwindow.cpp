#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyle>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->showMessage("就绪");

    // 工具栏图标:使用当前系统风格自带图标
    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));

    // 文件菜单栏
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);   // New
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile); // Open
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile); // Save
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);      // Exit

    // 编辑菜单栏:信号到来时才去取当前编辑器,没有标签页时什么也不做
    connect(ui->actionCut, &QAction::triggered, this, [this]()
            {if(QPlainTextEdit*e=currentEditor())e->cut(); });
    connect(ui->actionCopy, &QAction::triggered, this, [this]()
            {if(QPlainTextEdit*e=currentEditor())e->copy(); });
    connect(ui->actionPaste, &QAction::triggered, this, [this]()
            {if(QPlainTextEdit* e=currentEditor())e->paste(); });
    connect(ui->actionRedo, &QAction::triggered, this, [this]()
            {if (QPlainTextEdit *e = currentEditor())e->redo(); });
    connect(ui->actionUndo, &QAction::triggered, this, [this]()
            {if (QPlainTextEdit *e = currentEditor())e->undo(); });

    // 视图菜单栏

    // 帮助菜单栏

    // 标签页:用户点×号时发出请求,由onCloseTab决定怎么关
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onCloseTab);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewFile()
{
    m_untitledCount++;
    CodeEditor *newone = new CodeEditor;
    QString title = "未命名-" + QString::number(m_untitledCount);
    int idx = ui->tabWidget->addTab(newone, title); // 新增的加入标签页,由tabWidget接管
    ui->tabWidget->setCurrentIndex(idx);            // 自动切换到新标签
    newone->setFocus();                             // 光标放进编辑器内,可以直接打字
}

void MainWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(this, "打开文件", QString(), "C/C++ 文件 (*.c *.cpp *.h *.hpp);;所有文件 (*)"); // 静态函数,返回用户在弹出的文件选择框中选择的文件路径
    if (path.isEmpty())                                                                                                         // 逻辑:用户点了取消
        return;
    auto *editor = new CodeEditor;
    if (!editor->loadFile(path)) // 失败后new出来的CodeEditor编辑器没有放进tabWidget,没有parent,没人会释放它,要手动析构,注意内存泄漏
    {
        QMessageBox::warning(this, "打开失败", "无法打开文件:\n" + path);
        delete editor;
        return;
    }
    int idx = ui->tabWidget->addTab(editor, QFileInfo(path).fileName());
    ui->tabWidget->setTabToolTip(idx, path);
    ui->tabWidget->setCurrentIndex(idx);
    editor->setFocus();
}
void MainWindow::onSaveFile()
{
    // QString path = ;
    // auto *page = new QPlainTextEdit;
    // page->saveFile()
}

void MainWindow::onCloseTab(int index)
{
    QWidget *page = ui->tabWidget->widget(index); // 取出页面
    // TODO第二步,内容修改过时,弹出是否保存page
    ui->tabWidget->removeTab(index); // 这是从标签栏移除,页面对象还在
    page->deleteLater();             // 真正删除
}

CodeEditor *MainWindow::currentEditor()
{
    return qobject_cast<CodeEditor *>(ui->tabWidget->currentWidget());
}

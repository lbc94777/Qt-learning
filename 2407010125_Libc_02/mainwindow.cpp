#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyle>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->showMessage("就绪");

    // 工具栏图标:使用当前系统风格自带图标
    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::updateActions);

    // 文件菜单栏
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);   // New
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile); // Open
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile); // Save
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close); // Exit

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

    setupDocks(); // 放置好左右下的docks

    onNewFile();

    updateActions(); // 构造函数最后手动调用,让非法的动作按钮灰掉
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateActions()
{
    bool hasEditor = (currentEditor() != nullptr);
    ui->actionUndo->setEnabled(hasEditor);
    ui->actionRedo->setEnabled(hasEditor);
    ui->actionCut->setEnabled(hasEditor);
    ui->actionCopy->setEnabled(hasEditor);
    ui->actionPaste->setEnabled(hasEditor);
    ui->actionSave->setEnabled(hasEditor);
    ui->actionSaveAs->setEnabled(hasEditor);
}

bool MainWindow::saveEditor(CodeEditor *editor)
{
    if (editor->filePath().isEmpty())
        return saveEditorAs(editor);           // 未命名文件，先让用户选择保存位置
    if (!editor->saveFile(editor->filePath())) // 转给最底层,让CodeEditor(编辑器)与指定path配合写入编辑器内容
    {
        QMessageBox::warning(this, "保存失败", "无法写入文件：\n" + editor->filePath());
        return false;
    }
    ui->statusbar->showMessage("已保存：" + editor->filePath(), 2000);
    return true;
}
bool MainWindow::saveEditorAs(CodeEditor *e)
{
    QString path = QFileDialog::getSaveFileName(this, "另存为", e->filePath(),
                                                "C/C++ 文件 (*.c *.cpp *.h *.hpp);;所有文件 (*)");
    if (path.isEmpty())
        return false; // 用户点了取消，也算没有保存成功

    if (!e->saveFile(path))
    {
        QMessageBox::warning(this, "保存失败", "无法写入文件：\n" + path);
        return false;
    }

    updateTabTitle(e); // 文件名变了；如果之前没有改动，修改状态不变，不会触发信号，所以要手动更新
    ui->statusbar->showMessage("已保存：" + path, 2000);
    return true;
}
void MainWindow::onNewFile()
{
    m_untitledCount++;
    CodeEditor *e = new CodeEditor;
    e->setUntitledName("未命名-" + QString::number(m_untitledCount)); // 名字交给编辑器自己记住
    addEditorTab(e);
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
    addEditorTab(editor);
}
void MainWindow::onSaveFile()
{
    if (CodeEditor *e = currentEditor())
        saveEditor(e);
}

void MainWindow::onSaveAsFile()
{
    if (CodeEditor *e = currentEditor())
        saveEditorAs(e);
}

void MainWindow::onCloseTab(int index)
{
    CodeEditor *editor = qobject_cast<CodeEditor *>(ui->tabWidget->widget(index)); // 取出页面

    if (editor && !maybeSave(editor))
    {
        return;
    }
    QWidget *page = ui->tabWidget->widget(index); // 先取出页面，必须在 removeTab 之前
    ui->tabWidget->removeTab(index);              // 这是从标签栏移除,页面对象还在
    page->deleteLater();                          // 真正删除
}
CodeEditor *MainWindow::currentEditor()
{
    return qobject_cast<CodeEditor *>(ui->tabWidget->currentWidget());
}

void MainWindow::addEditorTab(CodeEditor *e)
{
    int idx = ui->tabWidget->addTab(e, QString()); // 标签先留空
    connect(e->document(), &QTextDocument::modificationChanged, this, [this, e]()
            { updateTabTitle(e); });
    updateTabTitle(e); // 必须在 addTab 之后，否则 indexOf 找不到它
    ui->tabWidget->setCurrentIndex(idx);
    e->setFocus();
}
void MainWindow::updateTabTitle(CodeEditor *e)
{
    int idx = ui->tabWidget->indexOf(e);
    if (idx < 0)
        return;
    QString title = e->displayName();
    if (e->document()->isModified())
    {
        title += " *";
    }
    ui->tabWidget->setTabText(idx, title); // 算出来的文字要设置到标签上才会显示
    ui->tabWidget->setTabToolTip(idx, e->filePath().isEmpty() ? e->displayName() : e->filePath());
}
bool MainWindow::maybeSave(CodeEditor *editor)
{
    ui->tabWidget->setCurrentWidget(editor); // 先切换过去，让用户看清问的是哪个文件
    if (editor->document()->isModified())
    {

        QMessageBox::StandardButton ret = QMessageBox::question(
            this, "保存修改",
            QString("「%1」已修改,是否保存?").arg(editor->displayName()), // 不用 tabText，它带着 *
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return false; // 取消,不关闭这个标签
        if (ret == QMessageBox::Save && !saveEditor(editor))
            return false; // 选了保存但没有保存成功：也不关闭，内容不能丢
                          // 走到这里说明：选了「不保存」，或者已经保存成功了
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        CodeEditor *e = qobject_cast<CodeEditor *>(ui->tabWidget->widget(i));
        if (e)
        {
            if (!maybeSave(e))
            {
                event->ignore();
                return;
            }
        }
    }
    event->accept();
}

void MainWindow::setupDocks()
{
    m_fileDock = new QDockWidget("文件树", this);
    m_fileDock->setObjectName("fileDock");
    m_fileDock->setWidget(new QLabel("这里以后放文件树"));
    addDockWidget(Qt::LeftDockWidgetArea, m_fileDock);

    m_infoDock = new QDockWidget("文件信息", this);
    m_infoDock->setObjectName("fileInfoDock");
    m_infoDock->setWidget(new QLabel("这里以后放文件信息"));
    addDockWidget(Qt::RightDockWidgetArea, m_infoDock);

    m_logDock = new QDockWidget("日志", this);
    m_logDock->setObjectName("logsDock");
    m_logDock->setWidget(new QLabel("这里以后放日志"));
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);

    ui->menuView->addAction(m_fileDock->toggleViewAction());
    ui->menuView->addAction(m_infoDock->toggleViewAction());
    ui->menuView->addAction(m_logDock->toggleViewAction());

    resizeDocks({m_fileDock, m_infoDock}, {220, 240}, Qt::Horizontal); // 左右两个的宽度
    resizeDocks({m_logDock}, {150}, Qt::Vertical);                     // 底部的高度
}

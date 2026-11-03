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
    /*
    1.得到路径
    2.无路径?提示用户选择路径
    3.另存为
    4.更新标签
    5.返回成功与否

    */

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
bool MainWindow::saveEditorAs(CodeEditor *editor)
{
    QString path = QFileDialog::getSaveFileName(this, "另存为", editor->filePath(),
                                                "C/C++ 文件 (*.c *.cpp *.h *.hpp);;所有文件 (*)");
    if (path.isEmpty())
        return false; // 用户点了取消，也算没有保存成功

    if (!editor->saveFile(path))
    {
        QMessageBox::warning(this, "保存失败", "无法写入文件：\n" + path);
        return false;
    }

    int idx = ui->tabWidget->indexOf(editor); // 按编辑器反查标签下标，不一定是当前页
    ui->tabWidget->setTabText(idx, QFileInfo(path).fileName());
    ui->tabWidget->setTabToolTip(idx, path);
    ui->statusbar->showMessage("已保存：" + path, 2000);
    return true;
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
    if (editor && editor->document()->isModified())
    {
        ui->tabWidget->setCurrentIndex(index); // 先切换过去，让用户看清问的是哪个文件

        QMessageBox::StandardButton ret = QMessageBox::question(
            this, "保存修改",
            QString("「%1」已修改,是否保存?").arg(ui->tabWidget->tabText(index)),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return; // 取消,不关闭这个标签
        if (ret == QMessageBox::Save && !saveEditor(editor))
            return; // 选了保存但没有保存成功：也不关闭，内容不能丢
        // 走到这里说明：选了「不保存」，或者已经保存成功了
    }
    QWidget *page = ui->tabWidget->widget(index); // 先取出页面，必须在 removeTab 之前
    ui->tabWidget->removeTab(index);              // 这是从标签栏移除,页面对象还在
    page->deleteLater();                          // 真正删除
}
CodeEditor *MainWindow::currentEditor()
{
    return qobject_cast<CodeEditor *>(ui->tabWidget->currentWidget());
}

// 文件:新建文件,关闭标签页,打开文件,保存文件,另存为,退出 编辑:撤销,重做,剪切,复制,粘贴
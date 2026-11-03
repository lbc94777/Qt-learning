#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "CodeEditor.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewFile();  // 新建文件
    void onOpenFile(); // 打开文件
    void onSaveFile(); // 保存文件
    void onSaveAsFile();
    void onCloseTab(int index); // 关闭标签页
    void updateActions();       // 根据有没有编辑器，启用或禁用动作
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    int m_untitledCount = 0;
    Ui::MainWindow *ui;
    CodeEditor *currentEditor();        // 得到当前标签页里面的编辑器,没有标签页时返回nullptr
    bool saveEditor(CodeEditor *e);     // 保存指定的编辑器，没有路径时转到另存为；成功返回 true
    bool saveEditorAs(CodeEditor *e);   // 让用户选择位置后保存；用户取消或保存失败返回 false
    void addEditorTab(CodeEditor *e);   // 把编辑器放进标签页,并连接修改信号
    void updateTabTitle(CodeEditor *e); // 设置标签的名字和修改状态
    bool maybeSave(CodeEditor *e);      // 对于要关闭修改过的文件,弹出选项框
};
#endif // MAINWINDOW_H

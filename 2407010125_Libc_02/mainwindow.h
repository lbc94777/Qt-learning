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
    void onNewFile();           // 新建文件
    void onOpenFile();          // 打开文件
    void onSaveFile();          // 保存文件
    void onCloseTab(int index); // 关闭标签页
    void updateActions();

private:
    int m_untitledCount = 0;
    Ui::MainWindow *ui;
    CodeEditor *currentEditor(); // 得到当前标签页里面的编辑器,没有标签页时返回nullptr
};
#endif // MAINWINDOW_H

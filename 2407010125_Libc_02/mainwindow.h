#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPlainTextEdit>
#include <QMainWindow>

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
    void onCloseTab(int index); // 关闭标签页

private:
    int m_untitledCount = 0;
    Ui::MainWindow *ui;
    QPlainTextEdit *currentEditor(); // 得到MainWindow内当前正打开的标签页
};
#endif // MAINWINDOW_H

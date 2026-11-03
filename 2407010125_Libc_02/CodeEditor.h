#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include <QPlainTextEdit>

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    QString filePath() const { return m_filePath; }
    bool loadFile(const QString &path); // 读取文件，成功返回 true
    bool saveFile(const QString &path); // 写入文件，成功返回 true
    void setUntitledName(const QString& name){m_untitledName=name;}
    QString displayName()const;//标签上显示的名字,有路径就是文件名,没有就是[未命名-n]
private:
    QString m_filePath; // 用这个path让editor指向一个文件路径,空字符串表示还没有保存过的「未命名」文件
    QString m_untitledName;//editor默认名字:没有路径时用的名字,若有路径则可随时得到,但得有个未命名的默认名字
};

#endif // CODEEDITOR_H

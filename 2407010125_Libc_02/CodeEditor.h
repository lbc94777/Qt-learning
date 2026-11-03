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

private:
    QString m_filePath; // 空字符串表示还没有保存过的「未命名」文件
};

#endif // CODEEDITOR_H

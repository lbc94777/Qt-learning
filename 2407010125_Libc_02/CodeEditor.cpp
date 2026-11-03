#include "CodeEditor.h"
#include <QFile>
#include <QTextStream>
#include <QFont>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
  this->setFont(QFont("Consolas", 11));          // 设置字体
  this->setLineWrapMode(QPlainTextEdit::NoWrap); // 长行横向滚动,不自动折行
}
bool CodeEditor::loadFile(const QString &path) // 逻辑:接收filepath,然后用内容填充编辑器,bool类型用来检验路径下的文件是否合法
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;
  QTextStream in(&file);
  QString content = in.readAll();
  this->setPlainText(content);
  m_filePath = path;
  this->document()->setModified(false);
  return true;
}
bool CodeEditor::saveFile(const QString &path)
{
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false; // 保存失败(打不开,没有写入权限或者不是text),返回false
  QTextStream out(&file);
  out << toPlainText(); // 先清空,再写入
  m_filePath = path;
  document()->setModified(false); // save后设回未修改过的状态
  return true;
}

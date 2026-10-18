#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QStringList>

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this); // 创建控件
    connect(ui->submitBtn, &QPushButton::clicked, this, &Widget::onSubmit);
    connect(ui->clearBtn, &QPushButton::clicked, this, &Widget::onClear);
};

Widget::~Widget()
{
    delete ui;
}

void Widget::onSubmit()
{
    QString name = ui->nameEdit->text().trimmed();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请输入姓名");
        return;
    }
    QString gender = ui->maleRadio->isChecked() ? "男" : "女";
    int age = ui->ageSpin->value();
    QString major = ui->majorCombo->currentText();

    QStringList hobbies;
    if (ui->readCheck->isChecked())
        hobbies << ui->readCheck->text();
    if (ui->sportCheck->isChecked())
        hobbies << ui->sportCheck->text();
    if (ui->musicCheck->isChecked())
        hobbies << ui->musicCheck->text();
    QString hobbyText = hobbies.isEmpty() ? "无" : hobbies.join(",");
    QString line = QString("%1 | %2 | %3岁 | %4 | 爱好:%5").arg(name).arg(gender).arg(age).arg(major).arg(hobbyText);
    ui->resultList->addItem(line);
}
void Widget::onClear() // 清空:把表单恢复成初始状态
{
    ui->nameEdit->clear(); // setText(nullptr)也行,能编译通过;
    ui->maleRadio->setChecked(true);
    ui->ageSpin->setValue(ui->ageSpin->minimum());
    ui->majorCombo->setCurrentIndex(0);
    ui->musicCheck->setChecked(false);
    ui->sportCheck->setChecked(false);
    ui->readCheck->setChecked(false);
    ui->nameEdit->setFocus(); // 光标放回姓名框
}

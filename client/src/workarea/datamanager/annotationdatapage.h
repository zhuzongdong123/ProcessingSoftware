#ifndef ANNOTATIONDATAPAGE_H
#define ANNOTATIONDATAPAGE_H

#include <QWidget>

namespace Ui {
class AnnotationDataPage;
}

class AnnotationDataPage : public QWidget
{
    Q_OBJECT

public:
    explicit AnnotationDataPage(QWidget *parent = nullptr);
    ~AnnotationDataPage();

private:
    Ui::AnnotationDataPage *ui;
};

#endif // ANNOTATIONDATAPAGE_H

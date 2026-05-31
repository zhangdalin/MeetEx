#ifndef COLLAPSIBLESECTION_H
#define COLLAPSIBLESECTION_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>

class CollapsibleSection : public QWidget {
    Q_OBJECT

public:
    explicit CollapsibleSection(const QString &title, QWidget *parent = nullptr);
    ~CollapsibleSection();

    void setContent(QWidget *content);
    QWidget* content() const;
    void setExpanded(bool expanded);
    bool isExpanded() const;
    void setSummary(const QString &summary);

signals:
    void expansionChanged(bool expanded);

private slots:
    void onToggleClicked();

private:
    void updateToggleIcon();
    void mousePressEvent(QMouseEvent *event) override;

    QToolButton *toggleButton_;
    QLabel *titleLabel_;
    QLabel *summaryLabel_;
    QWidget *contentContainer_;
    QWidget *contentWidget_;
    bool expanded_;
};

#endif // COLLAPSIBLESECTION_H

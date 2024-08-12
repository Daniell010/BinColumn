#ifndef BINCOLUMN_H
#define BINCOLUMN_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class BinColumn; }
QT_END_NAMESPACE

class BinColumn : public QMainWindow
{
    Q_OBJECT

public:
    BinColumn(QWidget *parent = nullptr);
    ~BinColumn();

private slots:
    void on_buttonOpenFile_clicked();
    void on_buttonApply_clicked();
    void on_buttonCancel_clicked();

private:
    Ui::BinColumn *ui;
    QList<quint16> listIndexes;

    QByteArray readCfgFile(const QString &fileName);
    QMap<quint8, QString> getHeaderFile();
    void showHeader();
    void fillingBox();
    QList<QStringList> parseCfgFile(const QByteArray &file);
    bool writeCfgFile(const QString &fileName, const QList<QStringList> &listLine);
    void processListLines(QList<QStringList> &listLine, const QString &numberSystem);
    QString generateNewFileName(const QString &fileName);
};
#endif // BINCOLUMN_H

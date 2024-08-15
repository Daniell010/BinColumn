#include <QFileDialog>
#include <QFile>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextStream>

#include "BinColumn.h"
#include "ui_BinColumn.h"

BinColumn::BinColumn(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BinColumn)
{
    ui->setupUi(this);

    ui->frameChoiceHandler->setVisible(false);
    ui->frameChoiceNS->setVisible(false);
    fillingBox();
}

BinColumn::~BinColumn()
{
    delete ui;
}

bool BinColumn::writeCfgFile(const QString &fileName, const QList<QStringList> &listLine)
{
    QFile fileToWrite(fileName);
    if (!fileToWrite.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        fileToWrite.errorString();
        return false;
    }

    QTextStream out(&fileToWrite);

    for (const auto &line : listLine)
    {
        out << line.join(",") << "\n";
    }

    fileToWrite.close();
    return true;
}

QByteArray BinColumn::readCfgFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        file.errorString();
        return QByteArray();
    }
    QByteArray fileData = file.readAll();
    file.close();

    return fileData;
}

QList<QStringList> BinColumn::parseCfgFile(const QByteArray &file)
{
    QList<QStringList> listLine;
    QTextStream stream(file);

    // Пропуск заголовка
    //QString header = stream.readLine();

    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        listLine.append(line.split(','));
    }

    return listLine;
}

QMap<quint8, QString> BinColumn::getHeaderFile()
{
    QFile file(ui->lineFileName->text());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        file.errorString();
        return QMap<quint8, QString>();
    }

    QTextStream in(&file);
    QString line = in.readLine();

    file.close();

    QStringList elements = line.split(",");
    QMap<quint8, QString> headerMap;

    for (quint8 i = 0; i < elements.size(); ++i)
    {
        headerMap.insert(i, elements.at(i).trimmed());
    }

    return headerMap;
}

void BinColumn::showHeader()
{
    QMap<quint8, QString> mapHeader = getHeaderFile();

    for (auto it =  mapHeader.begin(); it != mapHeader.end(); it++)
    {
        QPushButton *pButton = new QPushButton(this);
        pButton->setMinimumSize(QSize(0, 30));
        pButton->setMaximumSize(QSize(16777215, 30));
        pButton->setCheckable(true);
        pButton->setText(it.value());
        ui->verticalLayout->addWidget(pButton);

        quint16 key = it.key();
        connect(pButton, &QPushButton::clicked, [this, key, pButton]()
        {
            if (pButton->isChecked())
            {
                listIndexes.append(key);
            }
            else
            {
                listIndexes.removeOne(key);
            }

        });
    }
}

void BinColumn::on_buttonOpenFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*)"));
    if (!fileName.isEmpty())
    {
        ui->lineFileName->setText(fileName);
        ui->lineFileName->setToolTip(fileName);
        ui->frameChoiceHandler->setVisible(true);
        ui->frameChoiceNS->setVisible(true);
        showHeader();
    }
    else
    {
        ui->lineFileName->setText("");
        ui->lineFileName->setToolTip("");
        ui->frameChoiceHandler->setVisible(false);
        ui->frameChoiceNS->setVisible(false);
    }
}

void BinColumn::fillingBox()
{
    ui->boxNumberSystem->addItems(QStringList{"DEC", "HEX", "BIN", "OCT"});
}

void BinColumn::processListLines(QList<QStringList> &listLine, const QString &numberSystem)
{
    std::function<QString(int)> convertFunction;

    if (numberSystem == "DEC")
    {
        convertFunction = [](int number) { return QString::number(number); };
    }
    else if (numberSystem == "HEX")
    {
        convertFunction = [](int number) {  return QString("0x") + QString("%1").arg(number, 2, 16, QChar('0')).toUpper(); };
    }
    else if (numberSystem == "BIN")
    {
        convertFunction = [](int number) { return QString("0b%1").arg(number, 0, 2); };
    }
    else if (numberSystem == "OCT")
    {
        convertFunction = [](int number) { return QString("0%1").arg(number, 0, 8); };
    }
    else
    {
        return;
    }

    for (auto &line : listLine)
    {
        for (auto index : listIndexes)
        {
            if (line.size() > index)
            {
                bool ok = false;
                int number = line[index].toInt(&ok, 10);

                if (!ok) {
                    number = line[index].toInt(&ok, 16);
                }
                if (!ok) {
                    number = line[index].toInt(&ok, 8);
                }
                if (!ok) {
                    number = line[index].toInt(&ok, 2);
                }

                if (ok)
                {
                    line[index] = convertFunction(number);
                }
            }
        }
    }
}

QString BinColumn::generateNewFileName(const QString &fileName)
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), "");
    if (directory.isEmpty())
    {
        return QString();
    }

    int lastDotIndex = fileName.lastIndexOf('.');
    if (lastDotIndex == -1) return QString();

    int lastSlashIndex = fileName.lastIndexOf('/');

    QString baseName = fileName.mid(lastSlashIndex + 1, lastDotIndex - lastSlashIndex - 1);
    QString newBaseName = baseName + "_" + ui->boxNumberSystem->currentText() + ".csv";

    return directory + QDir::separator() + newBaseName;

}

void BinColumn::on_buttonApply_clicked()
{
    QString fileName = ui->lineFileName->text();

    QByteArray fileContent = readCfgFile(fileName);
    auto listLine = parseCfgFile(fileContent);

    processListLines(listLine, ui->boxNumberSystem->currentText());

    QString newFileName = generateNewFileName(fileName);

    if (!newFileName.isEmpty())
    {
        writeCfgFile(newFileName, listLine);
    }
}

void BinColumn::on_buttonCancel_clicked()
{
    listIndexes.clear();
    ui->frameChoiceHandler->setVisible(false);
    ui->frameChoiceNS->setVisible(false);
}

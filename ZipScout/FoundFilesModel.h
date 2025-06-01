#pragma once

#include <QAbstractTableModel>
#include <QVector>
#include <QString>
#include <QDateTime>

struct FoundFile {
    bool include = true;
    QString filePath;
    qint64 size = 0;
    QDateTime modified;
};

class FoundFilesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        COLUMN_INCLUDE = 0,
        COLUMN_FILENAME,
        COLUMN_SIZE,
        COLUMN_MODIFIED,
        COLUMN_COUNT
    };

    explicit FoundFilesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void clear();

    QStringList getCheckedFiles() const;

    void addFiles(const QVector<FoundFile>& files);

private:
    QVector<FoundFile> m_files;
};

#include "FoundFilesModel.h"
#include <QFileInfo>
#include <QIcon>

FoundFilesModel::FoundFilesModel(QObject *parent) 
    : QAbstractTableModel(parent)
{
}

int FoundFilesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_files.size();
}

int FoundFilesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : COLUMN_COUNT;
}

QVariant FoundFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_files.size())
        return QVariant();

    const FoundFile &file = m_files.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case COLUMN_FILENAME:
            return file.filePath;
        case COLUMN_SIZE:
            return QString::number(file.size);
        case COLUMN_MODIFIED:
            return file.modified.toString("yyyy-MM-dd HH:mm:ss");
        }
        break;
    case Qt::CheckStateRole:
        if (index.column() == COLUMN_INCLUDE)
            return file.include ? Qt::Checked : Qt::Unchecked;
        break;
    case Qt::TextAlignmentRole:
        if (index.column() == COLUMN_SIZE)
            return Qt::AlignRight;
        return Qt::AlignLeft;
    }

    return QVariant();
}

QVariant FoundFilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case COLUMN_INCLUDE: return tr("Include");
        case COLUMN_FILENAME: return tr("Filename");
        case COLUMN_SIZE: return tr("Size");
        case COLUMN_MODIFIED: return tr("Modified");
        }
    }
    return QVariant();
}

bool FoundFilesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_files.size())
        return false;

    if (index.column() == COLUMN_INCLUDE && role == Qt::CheckStateRole) {
        m_files[index.row()].include = value.toBool();
        emit dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    }

    return false;
}

Qt::ItemFlags FoundFilesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == COLUMN_INCLUDE)
        flags |= Qt::ItemIsUserCheckable;
    return flags;
}

void FoundFilesModel::sort(int column, Qt::SortOrder order)
{
    if (column == COLUMN_INCLUDE) {
        return;
    }

    beginResetModel();
    
    auto comparator = [column, order](const FoundFile &a, const FoundFile &b) {
        bool lessThan = false;
        
        switch (column) {
        case COLUMN_FILENAME:
            lessThan = a.filePath < b.filePath;
            break;
        case COLUMN_SIZE:
            lessThan = a.size < b.size;
            break;
        case COLUMN_MODIFIED:
            lessThan = a.modified < b.modified;
            break;
        }
        
        return order == Qt::AscendingOrder ? lessThan : !lessThan;
    };
    
    std::sort(m_files.begin(), m_files.end(), comparator);
    
    endResetModel();
}

void FoundFilesModel::clear()
{
    beginResetModel();
    m_files.clear();
    endResetModel();
}

QStringList FoundFilesModel::getCheckedFiles() const
{
    QStringList result;
    for (const FoundFile& file : m_files) {
        if (file.include) {
            result << file.filePath;
        }
    }
    return result;
}

void FoundFilesModel::addFiles(const QVector<FoundFile>& files)
{
    if (files.isEmpty()) return;

    beginInsertRows(QModelIndex(), m_files.size(), m_files.size() + files.size() - 1);
    m_files.append(files);
    endInsertRows();
}

/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: baijincheng <baijincheng@kylinos.cn>
 */
#include "fileoperation.h"
#include "mainwindow.h"

FileOperation::FileOperation(QObject *parent) : QObject(parent) {

}

/**
 * 获取文件存储的目录
 */
QString FileOperation::getRecordSaveDirectory(QString dirStr)
{
    QDir record;
    QLocale locale = QLocale::system().name();
    QDir recordDirectory;
    QString subDirectory = tr("Recorder");

    bool existEn = record.exists(dirStr + "Recorder");
    bool existCH = record.exists(dirStr + "录音");

    recordDirectory = QDir(dirStr);
    if (!existEn && !existCH) {
        if (locale.language() == QLocale::English) {
            recordDirectory.mkdir(subDirectory);
        } else if (locale.language() == QLocale::Chinese) {
            recordDirectory.mkdir(subDirectory);
        }
        return recordDirectory.filePath(subDirectory);
    } else {
        if (existEn) {
            return recordDirectory.filePath("Recorder");
        } else if (existCH) {
            return recordDirectory.filePath("录音");
        }
    }
    return recordDirectory.filePath(subDirectory);
}

/**
 * 文件信息列表
 */
QFileInfoList FileOperation::getRecordFileInfo()
{
    QFileInfoList fileInfoList;
    QStringList dirList = MainWindow::mutual->m_pathGSettings->get("recorderpath").toString().split(",");
    QStringList filters;
    filters << "*.pcm" << "*.wav" << "*.m4a";

    for (int i = 1; i < dirList.count(); i++) {
        QString dirStr = dirList.at(i);
        //将不同目录下的音频文件info统统加入fileInfoList
        fileInfoList << QDir(FileOperation::getRecordSaveDirectory(dirStr))
                            .entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    }
    return fileInfoList;
}

/**
 * 判断文件是否存在, 如果存在：它是否是一个文件而不是目录
 */
bool FileOperation::fileIsExist(QString path)
{
    QFileInfo check_file(path);

    return check_file.exists() && check_file.isFile();
}

QString FileOperation::formatMillisecond(int millisecond)
{
    if (millisecond / 1000 < 3600) {
        //至少需要返回1秒。
        return QDateTime::fromTime_t(std::max(1, millisecond / 1000)).toUTC().toString("hh:mm:ss");
    } else {
        return QDateTime::fromTime_t(millisecond / 1000).toUTC().toString("hh:mm:ss");
    }
}

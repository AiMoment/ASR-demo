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
#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QFileInfoList>
#include <QDateTime>

class FileOperation : public QObject
{
    Q_OBJECT
public:
    explicit FileOperation(QObject *parent = nullptr);

    QString getRecordSaveDirectory(QString dirStr);
    bool fileIsExist(QString path); // 判断文件是否存在
    QFileInfoList getRecordFileInfo();
    QString formatMillisecond(int millisecond);

};

#endif // FILEOPERATION_H

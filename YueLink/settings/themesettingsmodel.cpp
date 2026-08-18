/**
 * @file themesettingsmodel.cpp
 * @brief 实现外观设置的自动保存与变更通知。
 * @author xili <1424858143@qq.com>
 * @date 2026-08-17
 */

#include "themesettingsmodel.h"

#include "config/configapi.h"
#include "infrastructure/path.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QSet>
#include <QUuid>
#include <QVariantMap>
#include <QtGlobal>

#include <QsLog.h>

#include <algorithm>

ThemeSettingsModel::ThemeSettingsModel(QObject *parent)
: SettingsModelBase(parent)
{
    refreshFromConfig();
}

QString ThemeSettingsModel::mode() const
{
    return m_mode;
}

QString ThemeSettingsModel::primaryColor() const
{
    return m_primaryColor;
}

bool ThemeSettingsModel::animationsEnabled() const
{
    return m_animationsEnabled;
}

QUrl ThemeSettingsModel::backgroundImage() const
{
    return m_backgroundImage;
}

qreal ThemeSettingsModel::backgroundOpacity() const
{
    return m_backgroundOpacity;
}

QVariantList ThemeSettingsModel::backgroundImages() const
{
    return m_backgroundImages;
}

bool ThemeSettingsModel::updateMode(const QString &mode)
{
    const QString normalizedMode = mode.trimmed().toLower();
    if (m_mode == normalizedMode)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ThemeConfig>([&normalizedMode](Config::ThemeConfig &config) {
        config.mode = normalizedMode.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存主题模式：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存主题模式失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::updatePrimaryColor(const QString &color)
{
    const QColor parsedColor(color.trimmed());
    if (!parsedColor.isValid())
    {
        const QString error = tr("主题色格式无效，请使用十六进制颜色。");
        markError(error);
        return false;
    }

    const QString normalizedColor = parsedColor.name(QColor::HexRgb).toUpper();
    if (m_primaryColor == normalizedColor)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ThemeConfig>([&normalizedColor](Config::ThemeConfig &config) {
        config.primary_color = normalizedColor.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存主题色：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存主题色失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::updateAnimationsEnabled(bool enabled)
{
    if (m_animationsEnabled == enabled)
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ThemeConfig>([enabled](Config::ThemeConfig &config) {
        config.animations_enabled = enabled;
    });
    if (!result)
    {
        const QString error = tr("无法保存界面动画设置：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存界面动画设置失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::updateBackgroundImage(const QUrl &backgroundImage)
{
    QUrl normalizedUrl = backgroundImage.adjusted(QUrl::NormalizePathSegments);
    if (!normalizedUrl.isEmpty())
    {
        const bool isResource = normalizedUrl.scheme().compare(QStringLiteral("qrc"), Qt::CaseInsensitive) == 0;
        if (!normalizedUrl.isValid() || (!normalizedUrl.isLocalFile() && !isResource))
        {
            markError(tr("背景图片必须是本地文件或内置资源。"));
            return false;
        }

        const QString imagePath =
            normalizedUrl.isLocalFile() ? QFileInfo(normalizedUrl.toLocalFile()).absoluteFilePath() : QStringLiteral(":%1").arg(normalizedUrl.path());
        const QFileInfo imageInfo(imagePath);
        if (!imageInfo.exists() || !imageInfo.isFile() || QImageReader::imageFormat(imagePath).isEmpty())
        {
            markError(tr("所选文件不是可读取的图片。"));
            return false;
        }
        if (normalizedUrl.isLocalFile())
        {
            normalizedUrl = QUrl::fromLocalFile(imageInfo.absoluteFilePath());
        }
    }

    if (m_backgroundImage == normalizedUrl)
    {
        markSaved();
        return true;
    }

    const QString normalizedImage = normalizedUrl.toString(QUrl::FullyEncoded);
    const Config::Result result = Config::update<Config::ThemeConfig>([&normalizedImage](Config::ThemeConfig &config) {
        config.background_image = normalizedImage.toStdString();
    });
    if (!result)
    {
        const QString error = tr("无法保存背景图片：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存背景图片失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::updateBackgroundOpacity(qreal opacity)
{
    if (!qIsFinite(opacity) || opacity < 0.0 || opacity > 1.0)
    {
        markError(tr("背景透明度必须在 0% 到 100% 之间。"));
        return false;
    }
    if (qFuzzyCompare(m_backgroundOpacity + 1.0, opacity + 1.0))
    {
        markSaved();
        return true;
    }

    const Config::Result result = Config::update<Config::ThemeConfig>([opacity](Config::ThemeConfig &config) {
        config.background_opacity = opacity;
    });
    if (!result)
    {
        const QString error = tr("无法保存背景透明度：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 保存背景透明度失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::importBackgroundImage(const QString &name, const QUrl &sourceImage)
{
    const QString normalizedName = name.trimmed();
    if (normalizedName.isEmpty() || normalizedName.size() > 64)
    {
        markError(tr("背景名称必须包含 1 到 64 个字符。"));
        return false;
    }
    const Config::ThemeConfig currentConfig = Config::value<Config::ThemeConfig>();
    for (const Config::BackgroundImageConfig &background : currentConfig.background_images)
    {
        if (QString::fromStdString(background.name).compare(normalizedName, Qt::CaseInsensitive) == 0)
        {
            markError(tr("背景名称“%1”已存在。请使用其他名称。").arg(normalizedName));
            return false;
        }
    }

    const QUrl normalizedSource = sourceImage.adjusted(QUrl::NormalizePathSegments);
    if (!normalizedSource.isValid() || !normalizedSource.isLocalFile())
    {
        markError(tr("只能导入本地图片文件。"));
        return false;
    }

    const QFileInfo sourceInfo(normalizedSource.toLocalFile());
    const QByteArray imageFormat = QImageReader::imageFormat(sourceInfo.absoluteFilePath()).toLower();
    static const QSet<QByteArray> supportedFormats = {"png", "jpeg", "bmp", "webp"};
    if (!sourceInfo.exists() || !sourceInfo.isFile() || !supportedFormats.contains(imageFormat))
    {
        markError(tr("请选择可读取的 PNG、JPEG、BMP 或 WebP 图片。"));
        return false;
    }

    if (currentConfig.background_images.size() >= 64)
    {
        markError(tr("最多只能保存 64 张背景图片。"));
        return false;
    }

    const QString backgroundDirectory = Path::dataFile(QStringLiteral("backgrounds"));
    if (!QDir().mkpath(backgroundDirectory))
    {
        markError(tr("无法创建用户背景图片目录。"));
        return false;
    }

    const QString suffix = imageFormat == "jpeg" ? QStringLiteral("jpg") : QString::fromLatin1(imageFormat);
    const QString destinationPath = QDir(backgroundDirectory).filePath(QStringLiteral("%1.%2").arg(QUuid::createUuid().toString(QUuid::WithoutBraces), suffix));
    if (!QFile::copy(sourceInfo.absoluteFilePath(), destinationPath))
    {
        QFile::remove(destinationPath);
        markError(tr("无法将图片复制到用户背景库。"));
        return false;
    }

    const QString storedImage = QUrl::fromLocalFile(destinationPath).toString(QUrl::FullyEncoded);
    const Config::Result result = Config::update<Config::ThemeConfig>([&normalizedName, &storedImage](Config::ThemeConfig &config) {
        config.background_images.push_back(Config::BackgroundImageConfig{normalizedName.toStdString(), storedImage.toStdString(), true});
        config.background_image = storedImage.toStdString();
    });
    if (!result)
    {
        QFile::remove(destinationPath);
        const QString error = tr("无法保存用户背景图片：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 导入用户背景图片失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

bool ThemeSettingsModel::removeBackgroundImage(const QUrl &backgroundImage)
{
    const QUrl normalizedImage = backgroundImage.adjusted(QUrl::NormalizePathSegments);
    const bool isResource = normalizedImage.scheme().compare(QStringLiteral("qrc"), Qt::CaseInsensitive) == 0;
    if (!normalizedImage.isValid() || (!normalizedImage.isLocalFile() && !isResource))
    {
        markError(tr("所选背景图片地址无效。"));
        return false;
    }

    const Config::ThemeConfig currentConfig = Config::value<Config::ThemeConfig>();
    const auto enabledBackgroundCount =
        std::count_if(currentConfig.background_images.cbegin(), currentConfig.background_images.cend(), [](const Config::BackgroundImageConfig &background) {
            return background.enabled;
        });
    if (enabledBackgroundCount <= 1)
    {
        markError(tr("至少需要保留一张背景图片。"));
        return false;
    }
    const auto backgroundIterator = std::find_if(
        currentConfig.background_images.cbegin(), currentConfig.background_images.cend(), [&normalizedImage](const Config::BackgroundImageConfig &background) {
            return background.enabled && QUrl(QString::fromStdString(background.source)).adjusted(QUrl::NormalizePathSegments) == normalizedImage;
        });
    if (backgroundIterator == currentConfig.background_images.cend())
    {
        markError(tr("所选背景不在背景图片列表中。"));
        return false;
    }

    const auto fallbackIterator = std::find_if(
        currentConfig.background_images.cbegin(), currentConfig.background_images.cend(), [&normalizedImage](const Config::BackgroundImageConfig &background) {
            return background.enabled && QUrl(QString::fromStdString(background.source)).adjusted(QUrl::NormalizePathSegments) != normalizedImage;
        });
    if (fallbackIterator == currentConfig.background_images.cend())
    {
        markError(tr("至少需要保留一张背景图片。"));
        return false;
    }
    const QString fallbackImage = QString::fromStdString(fallbackIterator->source);
    const Config::Result result = Config::update<Config::ThemeConfig>([&normalizedImage, &fallbackImage](Config::ThemeConfig &config) {
        const auto backgroundToDisable =
            std::find_if(config.background_images.begin(), config.background_images.end(), [&normalizedImage](const Config::BackgroundImageConfig &background) {
                return background.enabled && QUrl(QString::fromStdString(background.source)).adjusted(QUrl::NormalizePathSegments) == normalizedImage;
            });
        if (backgroundToDisable != config.background_images.end())
        {
            backgroundToDisable->enabled = false;
        }
        if (QUrl(QString::fromStdString(config.background_image)).adjusted(QUrl::NormalizePathSegments) == normalizedImage)
        {
            config.background_image = fallbackImage.toStdString();
        }
    });
    if (!result)
    {
        const QString error = tr("无法删除背景图片：%1").arg(result.errorMessage);
        markError(error);
        QLOG_ERROR() << QStringLiteral("[设置] 删除背景图片失败 原因=") << result.errorMessage;
        return false;
    }

    refreshFromConfig();
    markSaved();
    return true;
}

void ThemeSettingsModel::refreshFromConfig()
{
    const Config::ThemeConfig config = Config::value<Config::ThemeConfig>();
    const QString mode = QString::fromStdString(config.mode);
    const QString primaryColor = QColor(QString::fromStdString(config.primary_color)).name(QColor::HexRgb).toUpper();
    const QUrl backgroundImage(QString::fromStdString(config.background_image));
    const qreal backgroundOpacity = config.background_opacity;
    QVariantList backgroundImages;
    backgroundImages.reserve(static_cast<qsizetype>(config.background_images.size()));
    for (const Config::BackgroundImageConfig &background : config.background_images)
    {
        if (!background.enabled)
        {
            continue;
        }
        backgroundImages.append(QVariantMap{{QStringLiteral("label"), QString::fromStdString(background.name)},
                                            {QStringLiteral("value"), QUrl(QString::fromStdString(background.source))}});
    }
    bool changed = false;

    if (m_mode != mode)
    {
        m_mode = mode;
        changed = true;
        emit modeChanged();
    }
    if (m_primaryColor != primaryColor)
    {
        m_primaryColor = primaryColor;
        changed = true;
        emit primaryColorChanged();
    }
    if (m_animationsEnabled != config.animations_enabled)
    {
        m_animationsEnabled = config.animations_enabled;
        changed = true;
        emit animationsEnabledChanged();
    }
    if (m_backgroundImage != backgroundImage)
    {
        m_backgroundImage = backgroundImage;
        changed = true;
        emit backgroundImageChanged();
    }
    if (!qFuzzyCompare(m_backgroundOpacity + 1.0, backgroundOpacity + 1.0))
    {
        m_backgroundOpacity = backgroundOpacity;
        changed = true;
        emit backgroundOpacityChanged();
    }
    if (m_backgroundImages != backgroundImages)
    {
        m_backgroundImages = backgroundImages;
        changed = true;
        emit backgroundImagesChanged();
    }
    if (changed)
    {
        emit settingsChanged();
    }
}

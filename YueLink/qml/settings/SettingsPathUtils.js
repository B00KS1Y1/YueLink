.pragma library

function localFilePath(fileUrl) {
    const fileUrlText = decodeURIComponent(fileUrl.toString());
    if (fileUrlText.startsWith("file:///"))
        return Qt.platform.os === "windows" ? fileUrlText.substring(8) : fileUrlText.substring(7);
    if (fileUrlText.startsWith("file://"))
        return "//" + fileUrlText.substring(7);
    return "";
}

function encodedPath(filePath) {
    return encodeURIComponent(filePath).replace(/%2F/gi, "/");
}

function localFileUrl(filePath) {
    const normalizedPath = filePath.trim().replace(/\\/g, "/");
    if (normalizedPath.startsWith("//"))
        return "file://" + encodedPath(normalizedPath.substring(2));
    if (/^[A-Za-z]:\//.test(normalizedPath))
        return "file:///" + normalizedPath.substring(0, 2) + encodedPath(normalizedPath.substring(2));
    if (normalizedPath.startsWith("/"))
        return "file://" + encodedPath(normalizedPath);
    return "";
}

function containingDirectoryPath(filePath) {
    const normalizedPath = filePath.trim().replace(/\\/g, "/");
    const separatorIndex = normalizedPath.lastIndexOf("/");
    if (/^[A-Za-z]:\//.test(normalizedPath) && separatorIndex === 2)
        return normalizedPath.substring(0, 3);
    if (separatorIndex === 0)
        return "/";
    return separatorIndex > 0 ? normalizedPath.substring(0, separatorIndex) : "";
}

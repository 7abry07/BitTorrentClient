#include "torrent.h"
#include <expected>

namespace btc::Torrent {

// ---------------------------------------------------
// ERROR
// ---------------------------------------------------

Error::Error(err_code code) : code(code) {}
std::string Error::get() { return err_mess.at(code); }

// ---------------------------------------------------
// TORRENT
// ---------------------------------------------------
Torrent::expected_val Torrent::validate(bencode_dict *root) {
  expected_bool announceRes = isAnnounceValid(root);
  if (!announceRes)
    return std::unexpected(announceRes.error());

  expected_bool infoRes = isInfoValid(root);
  if (!infoRes)
    return std::unexpected(infoRes.error());

  return Torrent(*root);
}

Torrent::expected_bool Torrent::isAnnounceValid(bencode_dict *root) {
  if (!root->contains("announce"))
    return std::unexpected(Error::missingAnnounceKeyErr);
  if (!root->at("announce").isStr())
    return std::unexpected(Error::announceKeyNotMappedToString);
  return true;
}

Torrent::expected_bool Torrent::isInfoValid(bencode_dict *root) {
  if (!root->contains("info"))
    return std::unexpected(Error::missingInfoKeyErr);
  if (!root->at("info").isDict())
    return std::unexpected(Error::infoKeyNotMappedToDictErr);

  expected_bool nameRes = isNameValid(&root->at("info").getDict());
  if (!nameRes)
    return std::unexpected(nameRes.error());

  expected_bool piecesRes = isPiecesValid(&root->at("info").getDict());
  if (!piecesRes)
    return std::unexpected(piecesRes.error());

  expected_bool pieceLengthRes =
      isPieceLengthValid(&root->at("info").getDict());
  if (!pieceLengthRes)
    return std::unexpected(pieceLengthRes.error());

  expected_bool lengthOrFilesRes =
      isLengthOrFilesValid(&root->at("info").getDict());
  if (!lengthOrFilesRes)
    return std::unexpected(lengthOrFilesRes.error());

  return true;
}

Torrent::expected_bool Torrent::isNameValid(bencode_dict *info) {
  if (!info->contains("name"))
    return std::unexpected(Error::missingNameFieldErr);
  if (!info->at("name").isStr())
    return std::unexpected(Error::nameFieldNotMappedToStrErr);
  return true;
}

Torrent::expected_bool Torrent::isPieceLengthValid(bencode_dict *info) {
  if (!info->contains("piece length"))
    return std::unexpected(Error::missingPieceLengthFieldErr);
  if (!info->at("piece length").isInt())
    return std::unexpected(Error::pieceLengthFieldNotMappedToIntErr);
  if (info->at("piece length").getInt() < 0)
    return std::unexpected(Error::pieceLengthNegativeErr);
  if (info->at("piece length").getInt() == 0)
    return std::unexpected(Error::pieceLengthZeroErr);
  return true;
}

Torrent::expected_bool Torrent::isPiecesValid(bencode_dict *info) {
  if (!info->contains("pieces"))
    return std::unexpected(Error::missingPiecesFieldErr);
  if (!info->at("pieces").isStr())
    return std::unexpected(Error::piecesFieldNotMapppedToStrErr);
  if (info->at("pieces").getStr().size() % 20 != 0)
    return std::unexpected(Error::piecesFieldLengthNonDivisibleBy20Err);
  return true;
}

Torrent::expected_bool Torrent::isLengthOrFilesValid(bencode_dict *info) {
  if (!info->contains("length") && !info->contains("files"))
    return std::unexpected(Error::bothLengthAndFilesFieldsMissingErr);
  if (info->contains("length") && info->contains("files"))
    return std::unexpected(Error::bothLengthAndFilesFieldsPresentErr);

  if (info->contains("length"))
    if (!info->at("length").isInt())
      return std::unexpected(Error::lengthFieldNotMappedToIntErr);

  if (info->contains("files")) {
    if (!info->at("files").isList())
      return std::unexpected(Error::filesFieldNotMappedToList);
    auto filesResult = isFilesListValid(&info->at("files").getList());
    if (!filesResult)
      return std::unexpected(filesResult.error());
  }
  return true;
}

Torrent::expected_bool Torrent::isFilesListValid(bencode_list *files) {
  for (auto items : *files) {
    if (!items.isDict())
      return std::unexpected(Error::filesFieldItemIsNotDictErr);

    if (!items.getDict().contains("length"))
      return std::unexpected(Error::missingLengthFieldInFilesItemErr);
    if (!items.getDict().at("length").isInt())
      return std::unexpected(Error::lengthFieldInFilesItemNotMappedToIntErr);

    if (!items.getDict().contains("path"))
      return std::unexpected(Error::missingPathFieldInFilesItemErr);
    if (!items.getDict().at("length").isStr())
      return std::unexpected(Error::pathFieldInFilesItemNotMappedToStrErr);
  }
  return true;
}

} // namespace btc::Torrent
  // namespace btc::Torrent

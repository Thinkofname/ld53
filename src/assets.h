#pragma once

namespace ld53::assets {
struct Tutorial {};
struct Tileset {
  struct Grass {};
  struct GrassWithStone {};

  struct TreeTop {};
  struct TreeBottom {};
  struct TreeBoth {};

  struct Mail {};
  struct Mailbox {};
  struct MailboxFull {};

  struct ButtonPlate {};
  struct ButtonPlatePressed {};
  struct Box {};

  struct Wall {};
  struct WallBottom {};
  struct Gate {};
  struct GateOpened {};

  struct PlayerIdleDown {};
  struct PlayerWalkDown {};
  struct PlayerIdleUp {};
  struct PlayerWalkUp {};
  struct PlayerIdleLeft {};
  struct PlayerWalkLeft {};
  struct PlayerIdleRight {};
  struct PlayerWalkRight {};

  struct WireTR {};
  struct WireTL {};
  struct WireBR {};
  struct WireBL {};
  struct WireTB {};
  struct WireLR {};
};

} // namespace ld53::assets

/*
MySQL Data Transfer
Source Host: localhost
Source Database: reality
Target Host: localhost
Target Database: reality
Date: 28.8.2010 6:26:56
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for characters
-- ----------------------------
CREATE TABLE `characters` (
  `charId` bigint(30) unsigned NOT NULL AUTO_INCREMENT,
  `userId` int(11) unsigned NOT NULL,
  `worldId` smallint(5) unsigned NOT NULL DEFAULT '1',
  `status` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'transit/banned',
  `handle` varchar(32) NOT NULL,
  `firstName` varchar(32) NOT NULL,
  `lastName` varchar(32) NOT NULL,
  `background` varchar(1024) DEFAULT NULL,
  `x` double NOT NULL DEFAULT '16802.3',
  `y` double NOT NULL DEFAULT '495',
  `z` double NOT NULL DEFAULT '3237.01',
  `rot` double NOT NULL DEFAULT '0.0245437',
  `healthC` mediumint(11) unsigned NOT NULL DEFAULT '500',
  `healthM` mediumint(11) unsigned NOT NULL DEFAULT '500',
  `innerStrC` mediumint(11) unsigned NOT NULL DEFAULT '200',
  `innerStrM` mediumint(11) unsigned NOT NULL DEFAULT '200',
  `level` mediumint(11) unsigned NOT NULL DEFAULT '50',
  `profession` int(10) unsigned NOT NULL DEFAULT '2',
  `alignment` smallint(6) unsigned NOT NULL DEFAULT '0',
  `pvpflag` smallint(6) unsigned NOT NULL DEFAULT '0',
  `exp` bigint(30) unsigned NOT NULL DEFAULT '1000000000',
  `cash` bigint(30) unsigned NOT NULL DEFAULT '10000',
  `district` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `adminFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `lastOnline` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `isOnline` tinyint(2) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`charId`),
  UNIQUE KEY `handle` (`handle`),
  UNIQUE KEY `charId` (`charId`)
) ENGINE=MyISAM AUTO_INCREMENT=357 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for doors
-- ----------------------------
CREATE TABLE `doors` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `doorid` int(11) unsigned NOT NULL,
  `districtid` tinyint(3) unsigned NOT NULL,
  `X` double NOT NULL DEFAULT '0',
  `Y` double NOT NULL DEFAULT '0',
  `Z` double NOT NULL DEFAULT '0',
  `ROT` double NOT NULL DEFAULT '0',
  `Open` bit(1) NOT NULL DEFAULT b'0',
  `OpenTime` datetime DEFAULT NULL,
  `DoorType` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `FirstUser` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=13168 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for hardlines
-- ----------------------------
CREATE TABLE `hardlines` (
  `Id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `HardLineId` smallint(6) unsigned NOT NULL,
  `HardlineName` varchar(45) NOT NULL,
  `X` double NOT NULL,
  `Y` double NOT NULL,
  `Z` double NOT NULL,
  `ROT` double NOT NULL,
  `DistrictId` smallint(6) unsigned NOT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Id` (`Id`)
) ENGINE=MyISAM AUTO_INCREMENT=139 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for inventory
-- ----------------------------
CREATE TABLE `inventory` (
  `invId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `charId` bigint(30) unsigned NOT NULL,
  `goid` int(11) unsigned NOT NULL,
  `slot` tinyint(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`invId`),
  UNIQUE KEY `invId` (`invId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for locations
-- ----------------------------
CREATE TABLE `locations` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Command` varchar(45) NOT NULL,
  `X` double NOT NULL,
  `Y` double NOT NULL,
  `Z` double NOT NULL,
  `District` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Id` (`Id`)
) ENGINE=MyISAM AUTO_INCREMENT=62 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for rsivalues
-- ----------------------------
CREATE TABLE `rsivalues` (
  `charId` bigint(30) unsigned NOT NULL,
  `sex` smallint(6) unsigned NOT NULL DEFAULT '0',
  `body` smallint(6) unsigned NOT NULL DEFAULT '2',
  `hat` smallint(6) unsigned NOT NULL DEFAULT '0',
  `face` smallint(6) unsigned NOT NULL DEFAULT '0',
  `shirt` smallint(6) unsigned NOT NULL DEFAULT '0',
  `coat` smallint(6) unsigned NOT NULL DEFAULT '0',
  `pants` smallint(6) unsigned NOT NULL DEFAULT '0',
  `shoes` smallint(6) unsigned NOT NULL DEFAULT '0',
  `gloves` smallint(6) unsigned NOT NULL DEFAULT '0',
  `glasses` smallint(6) unsigned NOT NULL DEFAULT '0',
  `hair` smallint(6) unsigned NOT NULL DEFAULT '0',
  `facialdetail` smallint(6) unsigned NOT NULL DEFAULT '0',
  `shirtcolor` smallint(6) unsigned NOT NULL DEFAULT '0',
  `pantscolor` smallint(6) unsigned NOT NULL DEFAULT '0',
  `coatcolor` smallint(6) unsigned NOT NULL DEFAULT '0',
  `shoecolor` smallint(6) unsigned NOT NULL DEFAULT '0',
  `glassescolor` smallint(6) unsigned NOT NULL DEFAULT '0',
  `haircolor` smallint(6) unsigned NOT NULL DEFAULT '0',
  `skintone` smallint(6) unsigned NOT NULL DEFAULT '0',
  `tattoo` smallint(6) unsigned NOT NULL DEFAULT '0',
  `facialdetailcolor` smallint(6) unsigned NOT NULL DEFAULT '0',
  `leggings` smallint(6) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`charId`),
  UNIQUE KEY `charId` (`charId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for users
-- ----------------------------
CREATE TABLE `users` (
  `userId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `passwordSalt` varchar(32) NOT NULL,
  `passwordHash` varchar(40) NOT NULL,
  `publicExponent` smallint(11) unsigned NOT NULL DEFAULT '0',
  `publicModulus` tinyblob,
  `privateExponent` tinyblob,
  `timeCreated` int(10) NOT NULL,
  `account_status` int(11) unsigned NOT NULL DEFAULT '1' COMMENT '0 = banned, 1 = user, 2 = admin',
  `sessionid` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`userId`),
  UNIQUE KEY `id` (`userId`),
  UNIQUE KEY `username` (`username`)
) ENGINE=MyISAM AUTO_INCREMENT=327 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for worlds
-- ----------------------------
CREATE TABLE `worlds` (
  `worldId` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(20) NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT '1 for no pvp, 2 for pvp',
  `status` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT 'World Status (Down, Open etc.)',
  `numPlayers` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`worldId`),
  UNIQUE KEY `worldId` (`worldId`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records 
-- ----------------------------
INSERT INTO `characters` VALUES ('35', '60', '1', '0', 'TestCharacter', 'Test', 'Character', 'Just some dude', '106181', '95', '-86876.8', '0.343612', '500', '500', '200', '200', '50', '2', '2', '0', '1000000000', '10000', '1', '1', '2010-08-28 06:11:29', '0');
INSERT INTO `characters` VALUES ('354', '60', '1', '0', 'TestChar2', 'Test2', 'Char2', 'Derperperper', '20897', '95', '50924.2', '-2.38074', '500', '500', '200', '200', '50', '2', '2', '0', '1000000000', '10000', '2', '0', '2010-08-26 03:26:16', '0');
INSERT INTO `doors` VALUES ('12849', '900726787', '1', '119700', '-505', '-50636', '0', '', null, '0', 'TestCharacter');
INSERT INTO `doors` VALUES ('12850', '211812355', '1', '115835', '-505', '-54298', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12851', '211812359', '1', '115835', '-505', '-55088.3', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12852', '211812358', '1', '115835', '-505', '-55292.1', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12853', '211812353', '1', '115835', '-505', '-56699.3', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12854', '211812357', '1', '115835', '-505', '-56882.4', '1.66897', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12855', '211812360', '1', '115835', '-505', '-57655', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12856', '211812369', '1', '114902', '-505', '-59165', '0.0981748', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12857', '211812977', '1', '114660', '-505', '-59882.2', '1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12858', '211812944', '1', '112660', '-505', '-61092.4', '1.64443', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12859', '211812362', '1', '113096', '-505', '-61740', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12860', '211812364', '1', '113292', '-505', '-61740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12861', '700448792', '1', '118297', '95', '-75165', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12862', '700450906', '1', '118504', '95', '-77140', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12863', '905969666', '1', '111902', '95', '-70035', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12864', '905969665', '1', '111697', '95', '-70035', '-3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12865', '905969668', '1', '111499', '95', '-70035', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12866', '905973238', '1', '110711', '95', '-69940', '0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12867', '905973486', '1', '109300', '95', '-70060', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12868', '905969672', '1', '108106', '95', '-70060', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12869', '905969669', '1', '108296', '95', '-70060', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12870', '905969671', '1', '108497', '95', '-70059.9', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12871', '905975253', '1', '107900', '95', '-70740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12872', '905974927', '1', '107940', '95', '-71900.4', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12873', '905974938', '1', '108297', '95', '-72340', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12874', '905974985', '1', '109504', '95', '-72860', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12875', '905975006', '1', '109881', '95', '-73340', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12876', '905975027', '1', '110504', '95', '-73340', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12877', '905974982', '1', '109460', '95', '-75499.3', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12878', '905974937', '1', '108298', '95', '-74060', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12879', '905974926', '1', '108060', '95', '-74509.2', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12880', '905975245', '1', '107260', '95', '-74298.2', '1.66897', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12881', '905977077', '1', '106301', '95', '-74540', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12882', '905977051', '1', '105702', '95', '-74060', '-3.06796', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12883', '905975246', '1', '107260', '95', '-71501.9', '1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12884', '905977031', '1', '105295', '95', '-71940', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12885', '905977011', '1', '104740', '95', '-73303.2', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12886', '905977070', '1', '106140', '95', '-73307.8', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12887', '905976998', '1', '104505', '95', '-71460', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12888', '905977040', '1', '105502', '95', '-71460', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12889', '1216348168', '1', '98103.8', '95', '-80835', '-3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12890', '1216348164', '1', '97703.9', '95', '-78260', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12891', '1216348162', '1', '97901.3', '95', '-78260', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12892', '1216348693', '1', '98340', '95', '-78910.3', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12893', '1216348163', '1', '98701.1', '95', '-78260', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12894', '1216348694', '1', '98340', '95', '-79507.7', '-1.66897', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12895', '1441792006', '1', '91801.2', '95', '-78165', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12896', '1441792007', '1', '91591.5', '95', '-78165', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12897', '1441792001', '1', '93208.4', '95', '-78260', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12898', '1441792005', '1', '93401', '95', '-78260', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12899', '1441792008', '1', '94197.4', '95', '-78260', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12900', '1441792053', '1', '91800.6', '95', '-80140', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12901', '1441792002', '1', '91595.7', '95', '-80740', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12902', '1441792028', '1', '93840', '95', '-80497.9', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12903', '1441792003', '1', '90805.6', '95', '-78260', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12904', '1441792012', '1', '89294.4', '95', '-78165', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12905', '1441792011', '1', '89095.4', '95', '-78166', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12906', '1441792136', '1', '88660', '95', '-78496.4', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12907', '1441792321', '1', '88295.3', '95', '-79740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12908', '1441792010', '1', '89098.1', '95', '-80740.1', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12909', '1375731745', '1', '90397.9', '95', '-84165', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12910', '1375733558', '1', '91196.9', '95', '-85140', '-0.0736311', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12911', '1375733562', '1', '90840', '95', '-86509.7', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12912', '1375731744', '1', '90390.3', '95', '-86740', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12913', '1375731742', '1', '90190.2', '95', '-86740', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12914', '1375733610', '1', '91201.9', '95', '-86260', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12915', '1375731750', '1', '89902.7', '95', '-89165.1', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12916', '1375733963', '1', '90705.3', '95', '-90140', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12917', '1375731749', '1', '89900.9', '95', '-91740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12918', '1375731747', '1', '89695.9', '95', '-91740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12919', '1375733967', '1', '90340', '95', '-91499', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12920', '1375734005', '1', '90700.2', '95', '-91260', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12921', '1375731751', '1', '90702.5', '95', '-91740', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12922', '1375731735', '1', '92200.2', '95', '-91835', '-3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12923', '1375731734', '1', '92404.9', '95', '-91834', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12924', '1375733003', '1', '92840', '95', '-91507.4', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12925', '1375733032', '1', '93196.3', '95', '-91260', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12926', '1375732999', '1', '93205.9', '95', '-90260', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12927', '1375731733', '1', '92404.3', '95', '-89260', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12928', '1375731722', '1', '94399.1', '95', '-89165', '0.0736311', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12929', '1375732519', '1', '94396.9', '95', '-89740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12930', '1375732510', '1', '94840', '95', '-91506.2', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12931', '1375732555', '1', '95210.3', '95', '-91260', '3.06796', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12932', '1375731725', '1', '94400.2', '95', '-91740', '0.0981748', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12933', '1375731723', '1', '94197.4', '95', '-91740', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12934', '1375731724', '1', '95198.7', '95', '-91835', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12935', '900726786', '1', '98635', '95', '-91302.2', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12936', '900727710', '1', '97859.8', '95', '-89293.2', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12937', '900727730', '1', '97060', '95', '-88294.4', '1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12938', '900727697', '1', '96298.1', '95', '-89740', '0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12939', '900727689', '1', '96495.3', '95', '-90340', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12940', '900727688', '1', '97499.6', '95', '-90340', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12941', '672137222', '1', '108965', '95', '-83501.8', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12942', '672137223', '1', '108965', '95', '-83707.1', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12943', '672137218', '1', '108965', '95', '-83902.1', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12944', '672139294', '1', '109060', '95', '-82695.6', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12945', '672139299', '1', '109740', '95', '-84094', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12946', '672139211', '1', '108940', '95', '-81308.8', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12947', '672137224', '1', '108940', '95', '-80116.3', '-1.69351', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12948', '672137221', '1', '108940', '95', '-80294.6', '-1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12949', '672137220', '1', '108940', '95', '-80486.7', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12950', '1427111938', '1', '121165', '95', '-69103.6', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12951', '1427111940', '1', '121165', '95', '-71097', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12952', '1427111987', '1', '121740', '95', '-69301.1', '-1.66897', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12953', '1427111939', '1', '123740', '95', '-68302.5', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12954', '1427111943', '1', '123740', '95', '-69101', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12955', '1427111942', '1', '123740', '95', '-69297.9', '-1.64443', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12956', '1427111937', '1', '123740', '95', '-70706', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12957', '1427111941', '1', '123740', '95', '-70908.4', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12958', '1427111944', '1', '123740', '95', '-71700.4', '-1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12959', '1427111952', '1', '123335', '95', '-67701.8', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12960', '1427111949', '1', '123335', '95', '-66895.2', '1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12961', '1427111945', '1', '123335', '95', '-66688.6', '1.49717', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12962', '1427111950', '1', '123335', '95', '-65307.5', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12963', '1427111951', '1', '123335', '95', '-65096.4', '1.47262', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12964', '1427111947', '1', '123335', '95', '-64296', '1.47262', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12965', '1427111946', '1', '120665', '95', '-65102.9', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12966', '1427111948', '1', '120665', '95', '-67109.4', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12967', '700448791', '1', '120297', '95', '-75165', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12968', '700448790', '1', '122302', '95', '-75165', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12969', '700448776', '1', '136301', '95', '-75165', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12970', '700448777', '1', '134302', '95', '-75165', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12971', '700449070', '1', '135697', '95', '-77340', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12972', '700449219', '1', '135000', '95', '-78308.4', '-1.64443', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12973', '700449095', '1', '134940', '-305', '-76900.5', '-1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12974', '700449130', '1', '134292', '-305', '-77140.1', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12975', '700449154', '1', '135490', '-305', '-78060', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12976', '700449040', '1', '134698', '495', '-76237.5', '-3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12977', '700449048', '1', '134290', '495', '-76140.2', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12978', '700449021', '1', '135579', '495', '-78492.8', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12979', '700449030', '1', '135313', '495', '-78200', '3.06796', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12980', '674234371', '1', '134299', '95', '-81165', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12981', '674234372', '1', '132303', '95', '-81165', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12982', '674234373', '1', '130296', '95', '-81165', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12983', '674235278', '1', '131701', '95', '-83660', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12984', '674235340', '1', '130502', '-305', '-82060', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12985', '1220542471', '1', '144700', '295', '-70835', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12986', '731906092', '1', '70164', '95', '-72710.5', '-1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12987', '731909801', '1', '70740', '95', '-72696.1', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12988', '731909205', '1', '70499.5', '95', '-72460', '-3.06796', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12989', '731906095', '1', '70260', '95', '-71700.1', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12990', '731906094', '1', '70260', '95', '-71892.2', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12991', '731909224', '1', '72340', '95', '-71499.4', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12992', '731909225', '1', '72340', '95', '-71301', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12993', '731909201', '1', '71740', '95', '-72704', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12994', '731906093', '1', '72740', '95', '-71898.5', '-1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12995', '731906098', '1', '72835', '95', '-73894.4', '1.49717', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12996', '731909554', '1', '71860', '95', '-74691.2', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12997', '731909558', '1', '70504.7', '95', '-74340', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12998', '731909827', '1', '70740', '95', '-74712.5', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('12999', '731906099', '1', '70260', '95', '-74701.7', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13000', '731906100', '1', '70260', '95', '-73888.6', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13001', '731906097', '1', '70260', '95', '-73698.4', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13002', '1007681544', '1', '67835', '95', '-80301.9', '1.47262', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13003', '1007681547', '1', '67835', '95', '-80106.2', '1.44808', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13004', '326107143', '1', '65905.9', '95', '-86565', '0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13005', '326107137', '1', '65697', '95', '-86565', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13006', '326107138', '1', '65498.1', '95', '-86565', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13007', '326107142', '1', '65297.4', '95', '-86565', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13008', '326108685', '1', '65596.9', '95', '-88340', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13009', '326108664', '1', '66140', '95', '-88711.1', '-1.64443', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13010', '326108709', '1', '65060', '95', '-88695.9', '1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13011', '774897665', '1', '59196.3', '95', '-87765', '0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13012', '774897667', '1', '58804.2', '95', '-87765', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13013', '774905352', '1', '58717', '95', '-91300', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13014', '774900248', '1', '60740', '95', '-91895.6', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13015', '774897666', '1', '62140', '95', '-91504.8', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13016', '774900222', '1', '60860', '95', '-93295.9', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13017', '774897670', '1', '59197.5', '95', '-94140', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13018', '774897669', '1', '58793.5', '95', '-94140', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13019', '774900223', '1', '57260', '95', '-93298.4', '1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13020', '774897668', '1', '55860', '95', '-91499.3', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13021', '774900247', '1', '57140', '95', '-91903.2', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13022', '681574423', '1', '58701.8', '95', '-83835', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13023', '681575853', '1', '58701.8', '95', '-83268.3', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13024', '681575807', '1', '58460', '95', '-83498.4', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13025', '681574421', '1', '57685.7', '95', '-83778.6', '-0.269981', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13026', '681574424', '1', '57893', '95', '-83705.3', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13027', '681575816', '1', '57907', '95', '-81860', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13028', '681574422', '1', '57896.1', '95', '-81200', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13029', '1007681549', '1', '59664', '95', '-80695.7', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13030', '1007685683', '1', '60240', '95', '-80703.2', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13031', '1007685655', '1', '60005.6', '95', '-80460', '-3.06796', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13032', '1007681552', '1', '59726.8', '95', '-79688.8', '1.49717', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13033', '1007681551', '1', '59737.8', '95', '-79891', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13034', '681574419', '1', '60200.6', '95', '-83835', '-3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13035', '681574418', '1', '60393.3', '95', '-83740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13036', '1007681555', '1', '58835', '95', '-80293.8', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13037', '1007681557', '1', '58835', '95', '-80093.8', '1.47262', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13038', '508559381', '1', '53105.4', '95', '-73335', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13039', '508560765', '1', '52302.4', '95', '-72360', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13040', '508559374', '1', '55602.6', '95', '-72835', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13041', '508559370', '1', '55606.6', '95', '-70260', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13042', '508559371', '1', '55797.1', '95', '-70260', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13043', '950009865', '1', '55297.4', '95', '-67635', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13044', '950009863', '1', '55088.9', '95', '-67635', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13045', '950009864', '1', '57290.6', '95', '-67635', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13046', '950009859', '1', '60709.3', '95', '-67635', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13047', '790626311', '1', '71465', '95', '-85205.3', '-1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13048', '790626309', '1', '71465', '95', '-84799.9', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13049', '790629545', '1', '74901.5', '95', '-83260', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13050', '790629547', '1', '77298.7', '95', '-83540', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13051', '790629593', '1', '77298.2', '95', '-86540', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13052', '790626307', '1', '79194.5', '95', '-88340', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13053', '790626312', '1', '78997.9', '95', '-88340', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13054', '1182793736', '1', '78897.8', '95', '-89165', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13055', '1182795025', '1', '79693.6', '95', '-90140', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13056', '1182793738', '1', '78896.6', '95', '-91740', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13057', '1182793735', '1', '78689.1', '95', '-91740', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13058', '241172484', '1', '81065', '95', '-98503.1', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13059', '790626313', '1', '79005', '95', '-81655', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13060', '790626310', '1', '79210', '95', '-81655', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13061', '498073615', '1', '82170', '95', '-77590', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13062', '498074708', '1', '84100', '95', '-77150', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13063', '498073607', '1', '84740', '95', '-76790', '-1.49717', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13064', '498073611', '1', '84740', '95', '-77600', '-1.52171', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13065', '498073612', '1', '84830', '95', '-77800', '1.71806', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13066', '498073610', '1', '84830', '95', '-79190', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13067', '498073608', '1', '84830', '95', '-79400', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13068', '498073609', '1', '84830', '95', '-80200', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13069', '790626308', '1', '83730', '95', '-84990', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13070', '224395267', '1', '86160', '95', '-88700', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13071', '731906088', '1', '75100', '95', '-76800', '3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13072', '731906103', '1', '72800', '95', '-76400', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13073', '731906105', '1', '70150', '95', '-76200', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13074', '731906104', '1', '70150', '95', '-76400', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13075', '731906102', '1', '70150', '95', '-77150', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13076', '731909842', '1', '70700', '95', '-77200', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13077', '731909767', '1', '70450', '95', '-76950', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13078', '1268776961', '1', '70700', '95', '-67830', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13079', '1268776963', '1', '69900', '95', '-67830', '-3.09251', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13080', '1268776964', '1', '69700', '95', '-67830', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13081', '1268777179', '1', '70700', '95', '-66260', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13082', '1268776962', '1', '69900', '95', '-65250', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13083', '1268776979', '1', '67890', '95', '-65160', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13084', '1268776978', '1', '65890', '95', '-65160', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13085', '1268776966', '1', '63890', '95', '-65160', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13086', '1268777482', '1', '63890', '95', '-65740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13087', '1268776969', '1', '63890', '95', '-67740', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13088', '1268776967', '1', '63690', '95', '-67740', '0.0736311', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13089', '353370118', '1', '64290', '95', '-70160', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13090', '353370120', '1', '64100', '95', '-70160', '0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13091', '353370122', '1', '63300', '95', '-70160', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13092', '353370385', '1', '63290', '95', '-70740', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13093', '353370114', '1', '65160', '95', '-71090', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13094', '353370351', '1', '65740', '95', '-71100', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13095', '353370113', '1', '67800', '95', '-71310', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13096', '353370116', '1', '67830', '95', '-71090', '1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13097', '353370115', '1', '67830', '95', '-70290', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13098', '508559364', '1', '61800', '95', '-70160', '-0.0736311', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13099', '508559363', '1', '61590', '95', '-70160', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13100', '508559361', '1', '60800', '95', '-70160', '0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13101', '508559394', '1', '60790', '95', '-70740', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13102', '508559395', '1', '61040', '95', '-70500', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13103', '1268776983', '1', '61700', '95', '-67830', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13104', '1268776982', '1', '61900', '95', '-67830', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13105', '1268776980', '1', '62700', '95', '-67830', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13106', '1268778881', '1', '62700', '95', '-67250', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13107', '1268778851', '1', '62460', '95', '-67490', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13108', '1268778847', '1', '62700', '95', '-66260', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13109', '1268776981', '1', '61900', '95', '-65260', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13110', '17825794', '1', '54830', '95', '-61890', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13111', '1302331425', '1', '54300', '95', '-55630', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13112', '1302332970', '1', '54340', '95', '-54300', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13113', '1302332912', '1', '55340', '95', '-53700', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13114', '1302331426', '1', '55690', '95', '-55540', '-0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13115', '1138753538', '1', '57660', '95', '-58700', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13116', '1138753540', '1', '57660', '95', '-58900', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13117', '1138753539', '1', '57660', '95', '-59700', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13118', '1138753794', '1', '58240', '95', '-59700', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13119', '1138753748', '1', '58000', '95', '-59460', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13120', '1138753757', '1', '59640', '95', '-58890', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13121', '1138753537', '1', '60240', '95', '-58900', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13122', '907018254', '1', '71830', '95', '-45290', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13123', '907018253', '1', '71830', '95', '-45090', '1.49717', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13124', '907018251', '1', '71830', '95', '-44290', '1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13125', '907018290', '1', '71260', '95', '-44280', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13126', '1268776993', '1', '61665', '95', '-61604.5', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13127', '1268779437', '1', '62640', '95', '-60803.4', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13128', '1268776992', '1', '64240', '95', '-61604.4', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13129', '1268776990', '1', '64240', '95', '-61809', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13130', '1268778586', '1', '64007.6', '95', '-61160', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13131', '1268778585', '1', '63760', '95', '-60794.7', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13132', '1268776994', '1', '64240', '95', '-60803.5', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13133', '34603031', '1', '57502.7', '694.999', '-35035', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13134', '34603030', '1', '57500.2', '694.999', '-34935', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13135', '34603025', '1', '57897.5', '694.999', '-35035', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13136', '34605960', '1', '57711.2', '694.999', '-34460', '-3.06796', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13137', '34605972', '1', '57319.1', '694.999', '-33260', '2.96979', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13138', '203423750', '1', '58691', '95', '-40565', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13139', '203423748', '1', '59303.8', '95', '-40565', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13140', '872415234', '1', '81165', '694.999', '-41094.3', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13141', '263192589', '1', '77335', '694.999', '-38899.7', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13142', '263192820', '1', '76002.4', '694.999', '-38660', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13143', '163577869', '1', '67704.1', '694.999', '-34034.9', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13144', '163577867', '1', '67904', '694.999', '-34035', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13145', '163577870', '1', '67493.5', '694.999', '-33940', '0', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13146', '163579156', '1', '67703.3', '694.999', '-33460', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13147', '163577871', '1', '64305.2', '694.999', '-34060', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13148', '163577866', '1', '64102.8', '694.999', '-34060', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13149', '163577872', '1', '64508.4', '694.999', '-34060', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13150', '163577861', '1', '67702.1', '694.999', '-26035', '3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13151', '163577859', '1', '67893.3', '694.999', '-25940', '-0.0981748', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13152', '163577862', '1', '67496.7', '694.999', '-25940', '-0.0490874', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13153', '430964742', '1', '83498.1', '694.999', '-34035', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13154', '430964741', '1', '83703.9', '694.999', '-34035', '-3.11705', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13155', '430964739', '1', '83900.2', '694.999', '-34035', '-3.14159', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13156', '900726787', '1', '99635', '95', '-88294.9', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13157', '900726790', '1', '99835', '95', '-81796.5', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13158', '900727743', '1', '99260', '95', '-81797.1', '1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13159', '900727744', '1', '99496.5', '95', '-82040', '0.0245437', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13160', '900726788', '1', '99740', '95', '-82802', '-1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13161', '900726791', '1', '99740', '95', '-82601.1', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13162', '900728053', '1', '97859.8', '95', '-82596.8', '1.5708', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13163', '900726789', '1', '97260', '95', '-82599.5', '1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13164', '900726795', '1', '97165.2', '95', '-84605.4', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13165', '900728249', '1', '98140', '95', '-83806', '-1.59534', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13166', '900726796', '1', '99740', '95', '-84606.4', '-1.61988', '', null, '1', 'TestCharacter');
INSERT INTO `doors` VALUES ('13167', '900726797', '1', '99740', '95', '-84804.6', '-1.54625', '', null, '1', 'TestCharacter');
INSERT INTO `hardlines` VALUES ('2', '49', 'MaraNorthWest', '7737.37', '95', '13801.5', '1.5708', '1');
INSERT INTO `hardlines` VALUES ('3', '152', 'MaraCentral', '17043.1', '495', '2398.8', '-3.14159', '1');
INSERT INTO `hardlines` VALUES ('6', '72', 'Tagged By Tastee Wheat', '39216.4', '95', '-21475.1', '2.03713', '1');
INSERT INTO `hardlines` VALUES ('7', '125', 'Tagged By Tastee Wheat', '25640', '-515', '-35813', '0.294524', '1');
INSERT INTO `hardlines` VALUES ('8', '73', 'Tagged By Tastee Wheat', '13211.7', '95', '-37821.4', '-2.72435', '2');
INSERT INTO `hardlines` VALUES ('9', '98', 'Tagged By Tastee Wheat', '9844.79', '1295', '1314.42', '-2.42983', '2');
INSERT INTO `hardlines` VALUES ('10', '48', 'Tagged By chefmaster', '-6415', '95', '-7121.87', '1.93895', '1');
INSERT INTO `hardlines` VALUES ('11', '146', 'Tagged By Tastee Wheat', '52941.3', '495', '41981.6', '-1.52171', '1');
INSERT INTO `hardlines` VALUES ('12', '113', 'Tagged By TheThirdMan', '-133308', '1295', '-35477', '1.44808', '2');
INSERT INTO `hardlines` VALUES ('13', '129', 'Tagged By TheThirdMan', '-2683.96', '-705', '31556.3', '2.82252', '1');
INSERT INTO `hardlines` VALUES ('14', '120', 'Tagged By IntegratedOne', '70083.9', '95', '44597.9', '-0.269981', '1');
INSERT INTO `hardlines` VALUES ('15', '116', 'Tagged By chefmaster', '-492.907', '95', '-31590.9', '0.343612', '1');
INSERT INTO `hardlines` VALUES ('16', '128', 'Tagged By Midnight', '54965.4', '95', '15087.4', '-0.0736311', '1');
INSERT INTO `hardlines` VALUES ('17', '78', 'Tagged By CloudWolf', '-49790.2', '95', '-159434', '-1.22718', '1');
INSERT INTO `hardlines` VALUES ('18', '147', 'Tagged By SanMarco', '111007', '-505', '-59842.7', '1.25173', '1');
INSERT INTO `hardlines` VALUES ('19', '136', 'Tagged By SanMarco', '111180', '95', '-40913.8', '1.7426', '1');
INSERT INTO `hardlines` VALUES ('20', '114', 'Tagged By SanMarco', '77349.4', '694.999', '-43966.8', '0.368155', '1');
INSERT INTO `hardlines` VALUES ('21', '55', 'Tagged By SanMarco', '82745.4', '694.999', '-66760.6', '-1.64443', '1');
INSERT INTO `hardlines` VALUES ('22', '130', 'Tagged By Agent', '51820.8', '95', '-42922', '-1.00629', '1');
INSERT INTO `hardlines` VALUES ('23', '138', 'Tagged By Superman111994', '-30111', '95', '-48204.8', '0', '1');
INSERT INTO `hardlines` VALUES ('24', '124', 'Tagged By SanMarco', '39584.6', '95', '-80338.7', '2.57709', '1');
INSERT INTO `hardlines` VALUES ('25', '107', 'Tagged By SanMarco', '63391.2', '95', '-91925.9', '0.220893', '1');
INSERT INTO `hardlines` VALUES ('26', '151', 'Tagged By Agent', '67566.8', '-105', '-10164.6', '-2.30711', '1');
INSERT INTO `hardlines` VALUES ('27', '132', 'Tagged By SanMarco', '96190.5', '95', '-85320.7', '3.01887', '1');
INSERT INTO `hardlines` VALUES ('28', '115', 'Tagged By SanMarco', '15657.9', '495', '-70054.5', '1.93895', '1');
INSERT INTO `hardlines` VALUES ('29', '112', 'Tagged By SanMarco', '40267.9', '95', '-116994', '-0.859029', '1');
INSERT INTO `hardlines` VALUES ('30', '104', 'Tagged By IntegratedOne', '-17757.8', '95', '-141682', '0.981748', '1');
INSERT INTO `hardlines` VALUES ('31', '122', 'Tagged By SanMarco', '10377', '95', '-110304', '2.67526', '1');
INSERT INTO `hardlines` VALUES ('32', '145', 'Tagged By SanMarco', '55512.5', '85', '-166913', '-1.20264', '1');
INSERT INTO `hardlines` VALUES ('33', '149', 'Tagged By SanMarco', '14659.5', '95', '-143258', '2.35619', '1');
INSERT INTO `hardlines` VALUES ('34', '113', 'Tagged By SanMarco', '107619', '-505', '-149092', '-0.0490874', '1');
INSERT INTO `hardlines` VALUES ('35', '148', 'Tagged By SanMarco', '79289.9', '-505', '-148965', '1.79169', '1');
INSERT INTO `hardlines` VALUES ('36', '111', 'Tagged By SanMarco', '87635.3', '85', '-117864', '1.42353', '1');
INSERT INTO `hardlines` VALUES ('37', '119', 'Tagged By Cookie', '-36326', '95', '-23953.1', '1.86532', '1');
INSERT INTO `hardlines` VALUES ('38', '109', 'Tagged By SanMarco', '135175', '95', '-103708', '-0.93266', '1');
INSERT INTO `hardlines` VALUES ('39', '76', 'Tagged By Cookie', '-67862.5', '95', '16314.2', '0.269981', '1');
INSERT INTO `hardlines` VALUES ('40', '150', 'Tagged By SanMarco', '111671', '-505', '-99321.3', '1.44808', '1');
INSERT INTO `hardlines` VALUES ('41', '153', 'Tagged By SanMarco', '12184.8', '-705', '59766', '-1.61988', '1');
INSERT INTO `hardlines` VALUES ('42', '131', 'Tagged By SanMarco', '100459', '95', '11859', '1.49717', '1');
INSERT INTO `hardlines` VALUES ('43', '142', 'Tagged By SanMarco', '94166.5', '95', '26721.2', '-2.40528', '1');
INSERT INTO `hardlines` VALUES ('44', '121', 'Tagged By SanMarco', '83610.7', '95', '57664.3', '2.99433', '1');
INSERT INTO `hardlines` VALUES ('45', '99', 'Tagged By Cookie', '-30434', '-705', '20325.5', '-0.981748', '1');
INSERT INTO `hardlines` VALUES ('46', '75', 'Tagged By Agent', '-64467.5', '-1105', '-57980.9', '3.01887', '1');
INSERT INTO `hardlines` VALUES ('47', '105', 'Tagged By Cookie', '-92910.8', '-705', '40713.9', '1.76715', '1');
INSERT INTO `hardlines` VALUES ('48', '101', 'Tagged By Superman111994', '-70778.6', '95', '-140154', '3.09251', '1');
INSERT INTO `hardlines` VALUES ('49', '106', 'Tagged By Agent', '-18499.8', '95', '-89853.4', '-0.490874', '1');
INSERT INTO `hardlines` VALUES ('50', '133', 'Tagged By Forez', '85775', '694.999', '-14502.5', '-1.81623', '1');
INSERT INTO `hardlines` VALUES ('51', '144', 'Tagged By Forez', '115650', '694.999', '4825', '-2.82252', '1');
INSERT INTO `hardlines` VALUES ('52', '12', 'Tagged By Othinn', '11039.9', '95', '35967.4', '0.0490874', '3');
INSERT INTO `hardlines` VALUES ('53', '10', 'Tagged By Othinn', '18310', '-505', '15226.4', '0.809942', '3');
INSERT INTO `hardlines` VALUES ('54', '102', 'Tagged By jayce77', '-86023.7', '95', '-77894.1', '1.81623', '1');
INSERT INTO `hardlines` VALUES ('55', '82', 'Tagged By Cyberkat', '14806.5', '1895', '72657.9', '1.52171', '2');
INSERT INTO `hardlines` VALUES ('56', '109', 'Tagged By Cyberkat', '3936.42', '1295', '96578.1', '1.5708', '2');
INSERT INTO `hardlines` VALUES ('57', '31', 'Tagged By Cyberkat', '-10586.3', '1895', '58667', '0.760854', '2');
INSERT INTO `hardlines` VALUES ('58', '105', 'Tagged By Cyberkat', '-19612.5', '694.999', '74764.7', '-1.81623', '2');
INSERT INTO `hardlines` VALUES ('59', '94', 'Tagged By Cyberkat', '-15563.7', '694.999', '95005.9', '2.9207', '2');
INSERT INTO `hardlines` VALUES ('60', '75', 'Tagged By Cyberkat', '-7704.53', '1895', '84346.2', '-0.981748', '2');
INSERT INTO `hardlines` VALUES ('61', '38', 'Tagged By Cyberkat', '-7466.52', '1895', '17801.8', '-0.0981748', '2');
INSERT INTO `hardlines` VALUES ('62', '56', 'Tagged By Cyberkat', '12047.9', '694.999', '17764.6', '-2.87161', '2');
INSERT INTO `hardlines` VALUES ('63', '92', 'Tagged By Cyberkat', '-23801.4', '1895', '12300.4', '1.76715', '2');
INSERT INTO `hardlines` VALUES ('64', '37', 'Tagged By Cyberkat', '-20669.7', '1895', '-19128', '-0.245437', '2');
INSERT INTO `hardlines` VALUES ('65', '19', 'Tagged By Agent', '-14261.6', '95', '9710.15', '-1.64443', '3');
INSERT INTO `hardlines` VALUES ('66', '14', 'Tagged By Agent', '-31420.5', '95', '8083.13', '-0.147262', '3');
INSERT INTO `hardlines` VALUES ('67', '15', 'Tagged By Agent', '-42980.2', '95', '9352.13', '1.84078', '3');
INSERT INTO `hardlines` VALUES ('68', '23', 'Tagged By Agent', '-65336.5', '95', '32209.4', '0.490874', '3');
INSERT INTO `hardlines` VALUES ('69', '17', 'Tagged By Agent', '36481.5', '-1105', '-30745', '-1.91441', '3');
INSERT INTO `hardlines` VALUES ('70', '7', 'Tagged By Agent', '42779.3', '-1105', '-5699.44', '-3.11705', '3');
INSERT INTO `hardlines` VALUES ('71', '20', 'Tagged By Agent', '79634.9', '-1115', '2825.15', '-2.79798', '3');
INSERT INTO `hardlines` VALUES ('72', '24', 'Tagged By Agent', '19783.5', '-505', '-11294.7', '-3.11705', '3');
INSERT INTO `hardlines` VALUES ('73', '6', 'Tagged By Agent', '15340.4', '95', '-32048.6', '-1.1781', '3');
INSERT INTO `hardlines` VALUES ('74', '16', 'Tagged By Agent', '-1445.26', '95', '-41080.3', '1.12901', '3');
INSERT INTO `hardlines` VALUES ('75', '5', 'Tagged By Agent', '-16244.9', '95', '-19746.3', '1.79169', '3');
INSERT INTO `hardlines` VALUES ('76', '11', 'Tagged By Agent', '-9540.12', '95', '-8841.22', '1.59534', '3');
INSERT INTO `hardlines` VALUES ('77', '13', 'Tagged By Agent', '-40011.6', '95', '-19769.5', '-2.84707', '3');
INSERT INTO `hardlines` VALUES ('78', '9', 'Tagged By Agent', '-48538.8', '95', '-31991.9', '0.0245437', '3');
INSERT INTO `hardlines` VALUES ('79', '21', 'Tagged By Agent', '-33794.6', '95', '-71125', '-0.368155', '3');
INSERT INTO `hardlines` VALUES ('80', '22', 'Tagged By Agent', '47043.9', '-1105', '-53683.6', '1.20264', '3');
INSERT INTO `hardlines` VALUES ('81', '18', 'Tagged By Agent', '24972', '-505', '33362.3', '-2.62618', '3');
INSERT INTO `hardlines` VALUES ('82', '3', 'Tagged By Agent', '42116.9', '-105', '57953', '2.94524', '3');
INSERT INTO `hardlines` VALUES ('83', '4', 'Tagged By Agent', '31450.7', '-505', '15775', '0.0736311', '3');
INSERT INTO `hardlines` VALUES ('84', '2', 'Tagged By Agent', '-37444.4', '95', '23659.1', '2.23348', '3');
INSERT INTO `hardlines` VALUES ('85', '8', 'Tagged By Agent', '-22193.8', '95', '32347.7', '3.04342', '3');
INSERT INTO `hardlines` VALUES ('86', '74', 'Tagged By Flerba', '42659.2', '95', '-2943.5', '0.908117', '2');
INSERT INTO `hardlines` VALUES ('87', '97', 'Tagged By SanMarco', '-148758', '694.999', '-49511.3', '-0.785398', '2');
INSERT INTO `hardlines` VALUES ('88', '78', 'Tagged By SanMarco', '-127692', '95', '-77862.7', '0.319068', '2');
INSERT INTO `hardlines` VALUES ('89', '101', 'Tagged By SanMarco', '-118336', '895', '-57827', '-1.42353', '2');
INSERT INTO `hardlines` VALUES ('90', '84', 'Tagged By SanMarco', '-161106', '95', '-78546.1', '0.539961', '2');
INSERT INTO `hardlines` VALUES ('91', '102', 'Tagged By SanMarco', '-151386', '-705', '-105035', '3.04342', '2');
INSERT INTO `hardlines` VALUES ('92', '89', 'Tagged By SanMarco', '-102034', '-705', '-104025', '-2.30711', '2');
INSERT INTO `hardlines` VALUES ('93', '79', 'Tagged By SanMarco', '-197691', '-705', '-106110', '2.23348', '2');
INSERT INTO `hardlines` VALUES ('94', '95', 'Tagged By SanMarco', '-213404', '95', '-83702.2', '2.45437', '2');
INSERT INTO `hardlines` VALUES ('95', '77', 'Tagged By SanMarco', '-101645', '1295', '-37998.6', '-3.06796', '2');
INSERT INTO `hardlines` VALUES ('96', '87', 'Tagged By SanMarco', '-92388.8', '295', '-52656.1', '-1.9635', '2');
INSERT INTO `hardlines` VALUES ('97', '108', 'Tagged By SanMarco', '-84171.4', '95', '-69825', '-0.834486', '2');
INSERT INTO `hardlines` VALUES ('98', '91', 'Tagged By SanMarco', '-51335.2', '95', '-26132.8', '-0.711767', '2');
INSERT INTO `hardlines` VALUES ('99', '96', 'Tagged By SanMarco', '-33636.6', '1895', '-45757.7', '0.294524', '2');
INSERT INTO `hardlines` VALUES ('100', '81', 'Tagged By SanMarco', '-2610.38', '1295', '-32441.3', '-1.27627', '2');
INSERT INTO `hardlines` VALUES ('101', '99', 'Tagged By SanMarco', '-38914.8', '95', '-58865.1', '0.46633', '2');
INSERT INTO `hardlines` VALUES ('102', '49', 'Tagged By SanMarco', '2945.95', '95', '-87486.3', '-1.93895', '2');
INSERT INTO `hardlines` VALUES ('103', '110', 'Tagged By SanMarco', '13888.2', '95', '-87848.4', '-1.88986', '2');
INSERT INTO `hardlines` VALUES ('104', '90', 'Tagged By SanMarco', '42918.6', '95', '-82035.5', '-0.687223', '2');
INSERT INTO `hardlines` VALUES ('105', '93', 'Tagged By SanMarco', '13797.4', '95', '-62540.9', '-0.589049', '2');
INSERT INTO `hardlines` VALUES ('106', '39', 'Tagged By SanMarco', '30494.8', '95', '-50434.3', '-2.01258', '2');
INSERT INTO `hardlines` VALUES ('107', '100', 'Tagged By SanMarco', '28299.5', '694.999', '-23233.1', '-2.06167', '2');
INSERT INTO `hardlines` VALUES ('108', '111', 'Tagged By SanMarco', '26340.3', '694.999', '-9418.3', '-2.9207', '2');
INSERT INTO `hardlines` VALUES ('109', '35', 'Tagged By SanMarco', '34943.9', '95', '9724.07', '0.908117', '2');
INSERT INTO `hardlines` VALUES ('110', '36', 'Tagged By SanMarco', '24184.7', '694.999', '3438.79', '-0.809942', '2');
INSERT INTO `hardlines` VALUES ('111', '48', 'Tagged By SanMarco', '-43576.8', '1295', '-236.303', '2.38074', '2');
INSERT INTO `hardlines` VALUES ('112', '63', 'Tagged By SanMarco', '-67676.3', '694.999', '-13554.3', '3.06796', '2');
INSERT INTO `hardlines` VALUES ('113', '59', 'Tagged By SanMarco', '-91392.6', '1295', '-13747.1', '0.785398', '2');
INSERT INTO `hardlines` VALUES ('114', '88', 'Tagged By SanMarco', '-77737.1', '1295', '-3403.01', '-1.44808', '2');
INSERT INTO `hardlines` VALUES ('115', '52', 'Tagged By SanMarco', '-62448', '1295', '15867.9', '2.20893', '2');
INSERT INTO `hardlines` VALUES ('116', '107', 'Tagged By SanMarco', '-57354.7', '1295', '33825', '-2.69981', '2');
INSERT INTO `hardlines` VALUES ('117', '50', 'Tagged By SanMarco', '-71709.4', '1295', '33189', '-0.613592', '2');
INSERT INTO `hardlines` VALUES ('118', '64', 'Tagged By SanMarco', '-70694', '1295', '40108', '-1.47262', '2');
INSERT INTO `hardlines` VALUES ('119', '85', 'Tagged By SanMarco', '-89310.2', '1295', '29097.4', '1.59534', '2');
INSERT INTO `hardlines` VALUES ('120', '80', 'Tagged By SanMarco', '-115777', '1295', '12425', '2.99433', '2');
INSERT INTO `hardlines` VALUES ('121', '112', 'Tagged By SanMarco', '-132482', '1295', '15399.5', '-3.06796', '2');
INSERT INTO `hardlines` VALUES ('122', '69', 'Tagged By SanMarco', '-145953', '694.999', '27883.6', '-3.09251', '2');
INSERT INTO `hardlines` VALUES ('123', '55', 'Tagged By SanMarco', '-142873', '694.999', '42221.6', '-1.42353', '2');
INSERT INTO `hardlines` VALUES ('124', '54', 'Tagged By SanMarco', '-127479', '694.999', '42677', '-1.98804', '2');
INSERT INTO `hardlines` VALUES ('125', '67', 'Tagged By SanMarco', '-140880', '694.999', '69638.8', '1.27627', '2');
INSERT INTO `hardlines` VALUES ('126', '47', 'Tagged By SanMarco', '-84175.4', '1295', '78182.1', '-2.72435', '2');
INSERT INTO `hardlines` VALUES ('127', '53', 'Tagged By SanMarco', '-44698.3', '694.999', '96080.3', '2.84707', '2');
INSERT INTO `hardlines` VALUES ('128', '86', 'Tagged By SanMarco', '-46627.5', '1895', '69787.3', '-1.54625', '2');
INSERT INTO `hardlines` VALUES ('129', '41', 'Tagged By SanMarco', '22764.8', '694.999', '42678.9', '-0.589049', '2');
INSERT INTO `hardlines` VALUES ('130', '106', 'Tagged By SanMarco', '22895.7', '694.999', '27604.9', '-1.20264', '2');
INSERT INTO `hardlines` VALUES ('131', '81', 'Tagged By AtraBile', '-38843.9', '95', '-1082.27', '1.15355', '1');
INSERT INTO `hardlines` VALUES ('132', '82', 'Tagged By Cookie', '-55135.7', '95', '-110187', '0.93266', '1');
INSERT INTO `hardlines` VALUES ('133', '100', 'Tagged By Maverick', '-75253.7', '95', '-7823.79', '-1.66897', '1');
INSERT INTO `hardlines` VALUES ('134', '103', 'Tagged By Tyndall', '-73303.1', '2', '-30083.3', '2.38074', '1');
INSERT INTO `hardlines` VALUES ('135', '80', 'Tagged By Tyndall', '-84633', '95', '8015.88', '-0.0245437', '1');
INSERT INTO `hardlines` VALUES ('136', '79', 'Tagged By Tyndall', '-94295.7', '-505', '-33925', '2.82252', '1');
INSERT INTO `hardlines` VALUES ('137', '43', 'Tagged By Tyndall', '-19925', '1895', '34694.1', '-1.88986', '2');
INSERT INTO `hardlines` VALUES ('138', '103', 'Tagged By megabeans', '-33575', '1895', '41507.1', '-1.64443', '2');
INSERT INTO `locations` VALUES ('17', 'Whiteroom', '203183', '95', '-172722', '1');
INSERT INTO `locations` VALUES ('18', 'Hallways', '211015', '-105', '-162254', '1');
INSERT INTO `locations` VALUES ('19', 'MaraCentral', '16802.3', '495', '3237.01', '1');
INSERT INTO `locations` VALUES ('20', 'VaultClub', '29966.4', '22295', '-3233.15', '2');
INSERT INTO `locations` VALUES ('21', 'Whiteroom2', '-58679.7', '95', '135827', '2');
INSERT INTO `locations` VALUES ('22', 'Whiteroom1', '-58418.9', '95', '139168', '2');
INSERT INTO `locations` VALUES ('23', 'LargeHallways1', '101672', '95', '99688.9', '2');
INSERT INTO `locations` VALUES ('24', 'LargeHallways2', '101600', '100', '126000', '2');
INSERT INTO `locations` VALUES ('25', 'PolyVinyl', '28464.5', '-705', '2111.1', '1');
INSERT INTO `locations` VALUES ('26', 'Neo', '-91572.5', '-705', '-164673', '1');
INSERT INTO `locations` VALUES ('27', 'CamonChurch', '111091', '-505', '-48948.7', '1');
INSERT INTO `locations` VALUES ('28', 'SanguineClub', '106321', '694.999', '-16772.9', '1');
INSERT INTO `locations` VALUES ('29', 'TaborPlaza', '48497.2', '495', '42309.5', '1');
INSERT INTO `locations` VALUES ('30', 'TaborTower', '51605.4', '7495', '53883', '1');
INSERT INTO `locations` VALUES ('31', 'KaltChemical', '81943', '95', '-107681', '1');
INSERT INTO `locations` VALUES ('32', 'DaemonClub', '-20607.7', '95', '-123654', '1');
INSERT INTO `locations` VALUES ('33', 'Caves', '-61657', '-3908.18', '-32561.3', '1');
INSERT INTO `locations` VALUES ('34', 'CentralPower', '-33059.7', '95', '-98468.7', '1');
INSERT INTO `locations` VALUES ('35', 'Probability', '-85534.3', '109.712', '-109513', '1');
INSERT INTO `locations` VALUES ('36', 'Ascension', '-49979.7', '2495', '-71401.1', '1');
INSERT INTO `locations` VALUES ('37', 'LinchpinClub', '-25615.2', '95', '-30282.1', '1');
INSERT INTO `locations` VALUES ('38', 'Mjolnir', '12105.5', '95', '-32503.2', '1');
INSERT INTO `locations` VALUES ('39', 'Highschool', '42018.5', '495', '-130268', '1');
INSERT INTO `locations` VALUES ('40', 'Majesty', '33480.9', '91.4854', '-146214', '1');
INSERT INTO `locations` VALUES ('41', 'ZalmonCasino', '104162', '-505', '-175606', '1');
INSERT INTO `locations` VALUES ('42', 'HammersfieldCourts', '130386', '-605', '-140205', '1');
INSERT INTO `locations` VALUES ('43', 'AzimuthTwin', '133007', '-108.184', '-98555.5', '1');
INSERT INTO `locations` VALUES ('44', 'AvalonClub', '125652', '95', '-116325', '1');
INSERT INTO `locations` VALUES ('45', 'Beryl', '140635', '3295', '-109089', '1');
INSERT INTO `locations` VALUES ('46', 'Office1', '5033.63', '495', '15395.4', '0');
INSERT INTO `locations` VALUES ('47', 'ExtractionRoom1', '9246.28', '295', '15059.3', '0');
INSERT INTO `locations` VALUES ('48', 'Subway1', '6846.27', '295', '21113.5', '0');
INSERT INTO `locations` VALUES ('49', 'ReadChairWhiteRoom1', '7492.12', '3995', '-15671', '0');
INSERT INTO `locations` VALUES ('50', 'Hypersphere', '-60925.3', '95', '12865.7', '1');
INSERT INTO `locations` VALUES ('51', 'HeartHotel', '-72165.7', '95', '5912.79', '1');
INSERT INTO `locations` VALUES ('52', 'Metacortex', '-124095', '1295', '-19459.8', '2');
INSERT INTO `locations` VALUES ('53', 'GovtBuilding', '68406.9', '1313', '27064.9', '2');
INSERT INTO `locations` VALUES ('54', 'TheLobby', '-67381.9', '1295', '26559.9', '2');
INSERT INTO `locations` VALUES ('55', 'ClubHel1', '-30507.6', '-14505', '-6885.32', '2');
INSERT INTO `locations` VALUES ('56', 'ClubHelBalcony1', '-27307.2', '-12905', '-563.901', '2');
INSERT INTO `locations` VALUES ('57', 'SphinxClub', '25676.4', '495', '-105156', '1');
INSERT INTO `locations` VALUES ('58', 'MaraNWHL', '7673.72', '95', '13844.9', '1');
INSERT INTO `locations` VALUES ('59', 'MaribeauTestBench', '-33684.5', '1895', '-45400.7', '2');
INSERT INTO `locations` VALUES ('60', 'TestTW', '186638.40625', '-905', '50007.699219', '1');
INSERT INTO `locations` VALUES ('61', 'LargeHalls1', '186638', '-905', '50007.7', '1');
INSERT INTO `rsivalues` VALUES ('35', '0', '2', '9', '7', '2', '10', '1', '6', '6', '4', '1', '6', '41', '16', '0', '0', '15', '1', '10', '0', '3', '0');
INSERT INTO `rsivalues` VALUES ('354', '1', '1', '1', '2', '7', '7', '1', '6', '7', '6', '1', '7', '0', '0', '1', '3', '5', '4', '3', '6', '2', '2');
INSERT INTO `users` VALUES ('60', 'loluser', 'JLEwx;+?', '5824550e13f2f150c479e14f4c4d0b7c838fdca8', '17', 0xA8A95456F64A3948E0F94F7E441A861970D3D9E7947DF5054382170D9D770A1EFD6C730FEDB41EB05AC46100BE74B3CC76D9B2AF68761E99B99CAB5096231EAD08E5354F0857F1E98864DE9C5B1FB7AB1E823EC3FACD3D0B6B427D42493E2C23, 0x123060F047BCB5DD315C242E3E8F68D08C2AECD535A9310313D44DC7BE2872085C95BC184E57307C6E2E46B246BC4595DFF74C3EA73EA39D889D80B99FF79C561923994617428E166C410D98F6C56A5818E922F485B4A9C2EF44F5BF56FF8E25, '1265666869', '2', null);
INSERT INTO `worlds` VALUES ('1', 'Reality', '1', '0', '0');

/*
MySQL Data Transfer
Source Host: localhost
Source Database: reality
Target Host: localhost
Target Database: reality
Date: 15.2.2010 13:58:17
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
  `x` double NOT NULL DEFAULT '16900',
  `y` double NOT NULL DEFAULT '500',
  `z` double NOT NULL DEFAULT '2800',
  `rot` double NOT NULL DEFAULT '0',
  `healthC` mediumint(11) NOT NULL DEFAULT '500',
  `healthM` mediumint(11) NOT NULL DEFAULT '500',
  `innerStrC` mediumint(11) NOT NULL DEFAULT '200',
  `innerStrM` mediumint(11) NOT NULL DEFAULT '200',
  `level` mediumint(11) NOT NULL DEFAULT '50',
  `profession` smallint(6) NOT NULL DEFAULT '2',
  `alignment` smallint(6) NOT NULL DEFAULT '0',
  `pvpflag` smallint(6) NOT NULL DEFAULT '0',
  `exp` bigint(30) NOT NULL DEFAULT '1000000000',
  `cash` bigint(30) NOT NULL DEFAULT '10000',
  `district` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`charId`),
  UNIQUE KEY `handle` (`handle`),
  UNIQUE KEY `charId` (`charId`)
) ENGINE=MyISAM AUTO_INCREMENT=36 DEFAULT CHARSET=latin1;

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
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for rsivalues
-- ----------------------------
CREATE TABLE `rsivalues` (
  `charId` bigint(30) unsigned NOT NULL,
  `sex` smallint(6) NOT NULL,
  `body` smallint(6) NOT NULL,
  `hat` smallint(6) NOT NULL,
  `face` smallint(6) NOT NULL,
  `shirt` smallint(6) NOT NULL,
  `coat` smallint(6) NOT NULL,
  `pants` smallint(6) NOT NULL,
  `shoes` smallint(6) NOT NULL,
  `gloves` smallint(6) NOT NULL,
  `glasses` smallint(6) NOT NULL,
  `hair` smallint(6) NOT NULL,
  `facialdetail` smallint(6) NOT NULL,
  `shirtcolor` smallint(6) NOT NULL,
  `pantscolor` smallint(6) NOT NULL,
  `coatcolor` smallint(6) NOT NULL,
  `shoecolor` smallint(6) NOT NULL,
  `glassescolor` smallint(6) NOT NULL,
  `haircolor` smallint(6) NOT NULL,
  `skintone` smallint(6) NOT NULL,
  `tattoo` smallint(6) NOT NULL,
  `facialdetailcolor` smallint(6) NOT NULL,
  `leggings` smallint(6) NOT NULL,
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
  `timeCreated` int(10) unsigned NOT NULL,
  `account_status` int(11) NOT NULL DEFAULT '0' COMMENT 'if banned',
  `sessionid` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`userId`),
  UNIQUE KEY `id` (`userId`),
  UNIQUE KEY `username` (`username`)
) ENGINE=MyISAM AUTO_INCREMENT=61 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for worlds
-- ----------------------------
CREATE TABLE `worlds` (
  `worldId` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(20) NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT '1 for no pvp, 2 for pvp',
  `status` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT 'World Status (Down, Open etc.)',
  `load` tinyint(3) unsigned NOT NULL DEFAULT '49',
  `numPlayers` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`worldId`),
  UNIQUE KEY `worldId` (`worldId`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records 
-- ----------------------------
INSERT INTO `characters` VALUES ('35', '60', '1', '0', 'TestCharacter', 'Test', 'Character', 'Just some dude', '37711.1', '95', '34252.4', '1.32536', '500', '500', '200', '200', '50', '2', '0', '0', '1000000000', '10000', '1');
INSERT INTO `rsivalues` VALUES ('35', '0', '2', '2', '5', '4', '6', '10', '7', '5', '18', '4', '6', '4', '2', '3', '6', '5', '4', '15', '0', '0', '0');
INSERT INTO `users` VALUES ('60', 'loluser', 'JLEwx;+?', '5824550e13f2f150c479e14f4c4d0b7c838fdca8', '17', 0xA8A95456F64A3948E0F94F7E441A861970D3D9E7947DF5054382170D9D770A1EFD6C730FEDB41EB05AC46100BE74B3CC76D9B2AF68761E99B99CAB5096231EAD08E5354F0857F1E98864DE9C5B1FB7AB1E823EC3FACD3D0B6B427D42493E2C23, 0x123060F047BCB5DD315C242E3E8F68D08C2AECD535A9310313D44DC7BE2872085C95BC184E57307C6E2E46B246BC4595DFF74C3EA73EA39D889D80B99FF79C561923994617428E166C410D98F6C56A5818E922F485B4A9C2EF44F5BF56FF8E25, '1265666869', '0', null);
INSERT INTO `worlds` VALUES ('1', 'Recursion', '1', '1', '49', '0');

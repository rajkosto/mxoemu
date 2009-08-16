/*
Navicat MySQL Data Transfer

Source Server         : localhost
Source Server Version : 50136
Source Host           : localhost:3306
Source Database       : reality

Target Server Type    : MYSQL
Target Server Version : 50136
File Encoding         : 65001

Date: 2009-08-16 16:38:24
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for `characters`
-- ----------------------------
DROP TABLE IF EXISTS `characters`;
CREATE TABLE `characters` (
  `charId` bigint(11) unsigned NOT NULL AUTO_INCREMENT,
  `userId` int(11) unsigned NOT NULL,
  `worldId` smallint(5) unsigned NOT NULL,
  `status` tinyint(3) unsigned NOT NULL COMMENT 'transit/banned',
  `handle` varchar(32) NOT NULL,
  `firstName` varchar(32) NOT NULL,
  `lastName` varchar(32) NOT NULL,
  `background` varchar(1024) DEFAULT NULL,
  PRIMARY KEY (`charId`),
  UNIQUE KEY `handle` (`handle`)
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of characters
-- ----------------------------
INSERT INTO `characters` VALUES ('2', '28', '1', '0', 'lolcharacter', 'lolman', 'matricks', 'I AM FROM DA MATRICKS YOU DO NOT MESS WITH ME');
INSERT INTO `characters` VALUES ('3', '28', '2', '0', 'TonkaTruck', 'Amanda', 'Mattricks', 'Not Like This.');
INSERT INTO `characters` VALUES ('4', '28', '3', '0', 'DrManhattan', 'Dr', 'Manhattan', 'THE BLUE MAN');
INSERT INTO `characters` VALUES ('5', '28', '2', '0', 'DerpMan', 'DurrrHurrr', 'Derp derp', 'HURRRRRRR');

-- ----------------------------
-- Table structure for `inventory`
-- ----------------------------
DROP TABLE IF EXISTS `inventory`;
CREATE TABLE `inventory` (
  `invId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `charId` bigint(11) unsigned NOT NULL,
  `goid` int(11) unsigned NOT NULL,
  `slot` tinyint(11) unsigned NOT NULL,
  PRIMARY KEY (`invId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of inventory
-- ----------------------------

-- ----------------------------
-- Table structure for `users`
-- ----------------------------
DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `userId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `passwordSalt` varchar(32) NOT NULL,
  `passwordHash` varchar(40) NOT NULL,
  `publicExponent` smallint(11) unsigned NOT NULL DEFAULT '0',
  `publicModulus` tinyblob,
  `privateExponent` tinyblob,
  `timeCreated` int(10) unsigned NOT NULL,
  `account_status` int(11) NOT NULL,
  `sessionid` varchar(100) NOT NULL,
  UNIQUE KEY `id` (`userId`),
  UNIQUE KEY `username` (`username`)
) ENGINE=MyISAM AUTO_INCREMENT=31 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of users
-- ----------------------------
INSERT INTO `users` VALUES ('28', 'loluser', 'yYygfF9c', '0b2825013f155cc11f7433780f54b9b412e2a52f', '17', 0xED7707AB64F87A51DD9020F73752E9455386A402BB539C64F8A4310EC211499DD503DA74187EE109C325CF07EC0A2FCF904C15CF38918B9782578D589294B6BC0033802EE099E8A8163D3A1D8E1A7B3238CE4B58A20C01B47180823C3ADC0B1D, 0x5ACBA10CD3C86B012F1176042BBDD1A990E0A81F2981D9EA5F119A496851E7710DAEA65990E537ED24F7DE37BC22033809A6D66CAB7205376859C0794708D71A6279537218A3D3496096AD23532179F1BB71315E64FFBEE609B98D8F08E05887, '1250413439', '1', '');
INSERT INTO `users` VALUES ('29', 'Shoo', '38urzWVw', 'c92d5eee7d21b71682052708c327e67996563be0', '0', null, null, '1250417299', '0', '246932749a403fed3c96e9949dfb3542');
INSERT INTO `users` VALUES ('30', 'Warboy', 'tZF1Umg4', 'd4c22f6a3d37dcc5b53f4789b9957e5b4f77b784', '0', null, null, '1250417422', '0', '4b48a65fa3f11f2cfa5564b1b3a1bb7d');

-- ----------------------------
-- Table structure for `worlds`
-- ----------------------------
DROP TABLE IF EXISTS `worlds`;
CREATE TABLE `worlds` (
  `worldId` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(20) NOT NULL,
  `type` tinyint(11) unsigned NOT NULL DEFAULT '1' COMMENT '1 for no pvp, 2 for pvp',
  `status` tinyint(11) unsigned NOT NULL DEFAULT '1' COMMENT 'World Status (Down, Open etc.)',
  `load` tinyint(3) unsigned NOT NULL DEFAULT '49',
  `numPlayers` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`worldId`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of worlds
-- ----------------------------
INSERT INTO `worlds` VALUES ('1', 'Recursion', '1', '1', '49', '0');
INSERT INTO `worlds` VALUES ('2', 'TestServer2', '1', '1', '49', '0');
INSERT INTO `worlds` VALUES ('3', 'HostileWorld1', '2', '1', '49', '0');
INSERT INTO `worlds` VALUES ('4', 'EnterTheMatricks', '1', '1', '49', '0');

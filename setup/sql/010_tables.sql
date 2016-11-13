USE `RN_development`;

-- Applications table
CREATE TABLE IF NOT EXISTS `rn_application` (
  `application_id` VARCHAR(32) NOT NULL COMMENT '',
  `description` TEXT NULL COMMENT '',
  `inserted_at` TIMESTAMP NULL DEFAULT current_timestamp COMMENT '',
  PRIMARY KEY (`application_id`)  COMMENT '',
  UNIQUE INDEX `application_id_UNIQUE` (`application_id` ASC)  COMMENT '');

-- Applications revisions
CREATE TABLE IF NOT EXISTS `rn_revision` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `application_id` VARCHAR(32) NOT NULL,
  `number` varchar(10) NOT NULL,
  `issued_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `apply_from` timestamp NULL DEFAULT NULL,
  `apply_to` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `application_id_idx` (`application_id`),
  CONSTRAINT `fk_application_id` FOREIGN KEY (`application_id`) REFERENCES `rn_application` (`application_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Per application policy definition
CREATE TABLE IF NOT EXISTS `rn_application_policy` (
   `application_id` VARCHAR(32) NOT NULL,
   `revisions_duration` int(11) NOT NULL DEFAULT '7200' COMMENT 'Duration of revisions in seconds.',
   `diversification_script` TEXT NULL COMMENT '',
   `timeout_mandatory` tinyint(1) NOT NULL DEFAULT '0',
   `start_from_revision` varchar(10) NOT NULL DEFAULT '0',
   `disable_renewability` TINYINT(1) NOT NULL DEFAULT '0',
   PRIMARY KEY (`application_id`),
   CONSTRAINT `fk1_application_id` FOREIGN KEY (`application_id`) REFERENCES `rn_application` (`application_id`) ON DELETE CASCADE ON UPDATE CASCADE
 ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
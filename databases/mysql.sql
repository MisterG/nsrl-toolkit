drop table if exists hash;
drop table if exists file;
drop table if exists product_has_os;
drop table if exists product;
drop table if exists os;
drop table if exists mfg;

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL';

CREATE  TABLE IF NOT EXISTS `nsrl`.`file` (
  `file_name` VARCHAR(256) NOT NULL ,
  `file_size` INT(10) UNSIGNED NOT NULL ,
  `product_code` INT(10) UNSIGNED NOT NULL ,
  `op_system_code` VARCHAR(15) NOT NULL ,
  `special_code` VARCHAR(10) NOT NULL ,
  `hash_sha1` BINARY(40) NOT NULL ,
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  INDEX `file_index_product_code` (`product_code` ASC) ,
  INDEX `file_index_op_system_code` (`op_system_code` ASC) ,
  INDEX `file_index_special_code` (`special_code` ASC) ,
  INDEX `file_index_hash_sha1` (`hash_sha1` ASC) ,
  PRIMARY KEY (`id`) ,
  CONSTRAINT `file_ibfk_1`
    FOREIGN KEY (`product_code` )
    REFERENCES `nsrl`.`product` (`product_code` )
    ON DELETE RESTRICT
    ON UPDATE RESTRICT,
  CONSTRAINT `file_ibfk_2`
    FOREIGN KEY (`op_system_code` )
    REFERENCES `nsrl`.`os` (`system_code` )
    ON DELETE RESTRICT
    ON UPDATE RESTRICT,
  CONSTRAINT `fk_file_1`
    FOREIGN KEY (`hash_sha1` )
    REFERENCES `nsrl`.`hash` (`sha1` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;

ALTER TABLE `nsrl`.`mfg` CHARACTER SET = utf8 ;

CREATE  TABLE IF NOT EXISTS `nsrl`.`os` (
  `system_code` VARCHAR(16) NOT NULL ,
  `system_name` VARCHAR(69) NOT NULL ,
  `system_version` VARCHAR(20) NOT NULL ,
  `mfg_code` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`system_code`) ,
  INDEX `fk_os_mfg_code` (`mfg_code` ASC) ,
  CONSTRAINT `os_fk_mfg`
    FOREIGN KEY (`mfg_code` )
    REFERENCES `nsrl`.`mfg` (`code` )
    ON DELETE RESTRICT
    ON UPDATE RESTRICT)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;

CREATE  TABLE IF NOT EXISTS `nsrl`.`product` (
  `product_code` INT(10) UNSIGNED NOT NULL ,
  `product_name` VARCHAR(138) NOT NULL ,
  `product_version` VARCHAR(49) NOT NULL ,
  `mfg_code` VARCHAR(45) NOT NULL ,
  `language` VARCHAR(64) NOT NULL ,
  `application_type` VARCHAR(36) NOT NULL ,
  PRIMARY KEY (`product_code`) ,
  INDEX `product_mfg_code_index` (`mfg_code` ASC) ,
  CONSTRAINT `product_fk_mfg`
    FOREIGN KEY (`mfg_code` )
    REFERENCES `nsrl`.`mfg` (`code` )
    ON DELETE RESTRICT
    ON UPDATE RESTRICT)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;

CREATE  TABLE IF NOT EXISTS `nsrl`.`product_has_os` (
  `product_code` INT(10) UNSIGNED NOT NULL ,
  `system_code` VARCHAR(16) NOT NULL ,
  PRIMARY KEY (`product_code`, `system_code`) ,
  INDEX `fk_product_has_os_os1` (`system_code` ASC) ,
  INDEX `fk_product_has_os_product1` (`product_code` ASC) ,
  CONSTRAINT `fk_product_has_os_product1`
    FOREIGN KEY (`product_code` )
    REFERENCES `nsrl`.`product` (`product_code` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_product_has_os_os1`
    FOREIGN KEY (`system_code` )
    REFERENCES `nsrl`.`os` (`system_code` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;

CREATE  TABLE IF NOT EXISTS `nsrl`.`hash` (
  `sha1` BINARY(40) NOT NULL ,
  `md5` BINARY(32) NOT NULL ,
  `crc32` VARCHAR(8) NULL ,
  INDEX `hash_index_sha1` (`sha1` ASC) )
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

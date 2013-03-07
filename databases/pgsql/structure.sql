-- Query to list the index owned by the user 'nsrl'
-- select c.relname, i.indisvalid, i.indisready from pg_index i, pg_class c, pg_roles r where i.indisprimary = false and i.indexrelid = c.oid and c.reltype = 0 and c.relowner = r.oid and r.rolname = 'nsrl';
BEGIN;

DROP TABLE IF EXISTS hash CASCADE;
DROP TABLE IF EXISTS file CASCADE;
DROP TABLE IF EXISTS product_has_os CASCADE;
DROP TABLE IF EXISTS product CASCADE;
DROP TABLE IF EXISTS os CASCADE;
DROP TABLE IF EXISTS mfg CASCADE;

DROP FUNCTION IF EXISTS upsert_hash(BYTEA, BYTEA, VARCHAR(8));
DROP FUNCTION IF EXISTS upsert_file(VARCHAR(256), BIGINT, BIGINT, VARCHAR(15), VARCHAR(10), BYTEA);
DROP FUNCTION IF EXISTS upsert_product(in_product_code BIGINT, in_product_name VARCHAR(138), in_product_version VARCHAR(49), in_mfg_code VARCHAR(45), in_language VARCHAR(64), in_application_type VARCHAR(36));
DROP FUNCTION IF EXISTS upsert_product_has_os(BIGINT, VARCHAR(16));
DROP FUNCTION IF EXISTS upsert_os(VARCHAR(16), VARCHAR(69), VARCHAR(20), VARCHAR(45));
DROP FUNCTION IF EXISTS upsert_mfg(varchar(45), varchar(45));

DROP INDEX hash_index_sha1;
DROP INDEX product_mfg_code_index;
DROP INDEX fk_os_mfg_code;
DROP INDEX file_index_product_code;
DROP INDEX file_index_op_system_code;
DROP INDEX file_index_special_code;
DROP INDEX file_index_hash_sha1;
DROP INDEX fk_product_has_os_os;
DROP INDEX fk_product_has_os_product;

CREATE TABLE IF NOT EXISTS mfg (
  code varchar(45) NOT NULL,
  name varchar(54) NOT NULL,
  PRIMARY KEY (code)
);

CREATE FUNCTION upsert_mfg(in_code varchar(45), in_name varchar(45)) RETURNS VOID AS
$$
BEGIN
	PERFORM code FROM mfg WHERE code = in_code;

	IF found THEN
		UPDATE mfg SET (name) = (in_name) WHERE code = in_code;
		RETURN;
	END IF;

	BEGIN
		INSERT INTO mfg (code, name) VALUES (in_code, in_name);
	END;
END;
$$
LANGUAGE plpgsql;

CREATE TABLE IF NOT EXISTS hash (
  sha1 BYTEA NOT NULL,
  md5 BYTEA NOT NULL,
  crc32 VARCHAR(8) NULL,
  PRIMARY KEY (sha1)
);
CREATE INDEX hash_index_sha1 ON hash (sha1);

CREATE FUNCTION upsert_hash(in_sha1 BYTEA, in_md5 BYTEA, in_crc32 VARCHAR(8)) RETURNS VOID AS
$$
BEGIN
	PERFORM sha1 FROM hash WHERE sha1 = in_sha1;

	IF found THEN
		UPDATE hash SET (md5, crc32) = (in_md5, in_crc32) WHERE sha1 = in_sha1;
		RETURN;
	END IF;

	BEGIN
		INSERT INTO hash (sha1, md5, crc32) VALUES (in_sha1, in_md5, in_crc32);
	END;
END;
$$
LANGUAGE plpgsql;

-- TODO: create another table for the supported languages
CREATE TABLE IF NOT EXISTS product (
  product_code BIGINT NOT NULL ,
  product_name VARCHAR(150) NOT NULL ,
  product_version VARCHAR(49) NOT NULL ,
  mfg_code VARCHAR(45) NOT NULL ,
  language VARCHAR(256) NOT NULL ,
  application_type VARCHAR(128) NOT NULL ,
  PRIMARY KEY (product_code) ,
  CONSTRAINT product_fk_mfg
    FOREIGN KEY (mfg_code )
    REFERENCES mfg (code )
    ON DELETE RESTRICT
    ON UPDATE RESTRICT
);
CREATE INDEX product_mfg_code_index ON product (mfg_code);

CREATE FUNCTION upsert_product(in_product_code BIGINT, in_product_name VARCHAR(138), in_product_version VARCHAR(49), in_mfg_code VARCHAR(45), in_language VARCHAR(64), in_application_type VARCHAR(36)) RETURNS VOID AS
$$
BEGIN
	PERFORM product_code FROM product WHERE product_code = in_product_code;

	IF found THEN
		UPDATE product SET (product_name, product_version, mfg_code, language, application_type) = (in_product_name, in_product_version, in_mfg_code, in_language, in_application_type) WHERE product_code = in_product_code;
		RETURN;
	END IF;

	BEGIN
		INSERT INTO product (product_code, product_name, product_version, mfg_code, language, application_type) VALUES (in_product_code, in_product_name, in_product_version, in_mfg_code, in_language, in_application_type);
	END;
END;
$$
LANGUAGE plpgsql;

CREATE TABLE IF NOT EXISTS os (
  system_code VARCHAR(16) NOT NULL ,
  system_name VARCHAR(69) NOT NULL ,
  system_version VARCHAR(20) NOT NULL ,
  mfg_code VARCHAR(45) NOT NULL ,
  PRIMARY KEY (system_code) ,
  CONSTRAINT os_fk_mfg
    FOREIGN KEY (mfg_code )
    REFERENCES mfg (code )
    ON DELETE RESTRICT
    ON UPDATE RESTRICT
);
CREATE INDEX fk_os_mfg_code ON os (mfg_code);

CREATE FUNCTION upsert_os(in_system_code VARCHAR(16), in_system_name VARCHAR(69), in_system_version VARCHAR(20), in_mfg_code VARCHAR(45)) RETURNS VOID AS
$$
BEGIN
	PERFORM system_code FROM os WHERE system_code = in_system_code;

	IF found THEN
		UPDATE os SET (system_name, system_version, mfg_code) = (in_system_name, in_system_version, in_mfg_code) WHERE system_code = in_system_code;
		RETURN;
	END IF;

	BEGIN
		INSERT INTO os (system_code, system_name, system_version, mfg_code) VALUES (in_system_code, in_system_name, in_system_version, in_mfg_code);
	END;
END;
$$
LANGUAGE plpgsql;

CREATE TABLE IF NOT EXISTS file (
  file_name VARCHAR(256) NOT NULL,
  file_size BIGINT NOT NULL,
  product_code BIGINT NOT NULL,
  op_system_code VARCHAR(15) NOT NULL,
  special_code VARCHAR(10) NOT NULL,
  hash_sha1 BYTEA NOT NULL,
  id SERIAL,
  PRIMARY KEY (id),
  CONSTRAINT file_ibfk_1
    FOREIGN KEY (product_code)
    REFERENCES product (product_code)
    ON DELETE RESTRICT
    ON UPDATE RESTRICT,
  CONSTRAINT file_ibfk_2
    FOREIGN KEY (op_system_code )
    REFERENCES os (system_code )
    ON DELETE RESTRICT
    ON UPDATE RESTRICT,
  CONSTRAINT fk_file_1
    FOREIGN KEY (hash_sha1)
    REFERENCES hash (sha1)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION
);
CREATE INDEX file_index_product_code ON file (product_code);
CREATE INDEX file_index_op_system_code ON file (op_system_code);
CREATE INDEX file_index_special_code ON file (special_code);
CREATE INDEX file_index_hash_sha1 ON file (hash_sha1);

CREATE FUNCTION upsert_file(in_file_name VARCHAR(256), in_file_size BIGINT, in_product_code BIGINT, in_op_system_code VARCHAR(15), in_special_code VARCHAR(10), in_hash_sha1 BYTEA) RETURNS VOID AS
$$
BEGIN
	PERFORM id FROM file WHERE file_name = in_file_name AND file_size = in_file_size AND product_code = in_product_code AND op_system_code = in_op_system_code AND special_code = in_special_code AND hash_sha1 = in_hash_sha1;

	IF found THEN
		RETURN;
	END IF;

	BEGIN
		INSERT INTO file (file_name, file_size, product_code, op_system_code, special_code, hash_sha1) VALUES (in_file_name, in_file_size, in_product_code, in_op_system_code, in_special_code, in_hash_sha1);
	END;
END;
$$
LANGUAGE plpgsql;

CREATE TABLE IF NOT EXISTS product_has_os (
  product_code BIGINT NOT NULL ,
  system_code VARCHAR(16) NOT NULL ,
  PRIMARY KEY (product_code, system_code) ,
  CONSTRAINT fk_product_has_os_product1
    FOREIGN KEY (product_code )
    REFERENCES product (product_code )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT fk_product_has_os_os1
    FOREIGN KEY (system_code )
    REFERENCES os (system_code )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION
);
CREATE INDEX fk_product_has_os_os ON product_has_os (system_code);
CREATE INDEX fk_product_has_os_product ON product_has_os (product_code );

CREATE FUNCTION upsert_product_has_os(in_product_code BIGINT, in_system_code VARCHAR(16)) RETURNS VOID AS
$$
BEGIN
	PERFORM product_has_os FROM product_has_os WHERE product_code = in_product_code AND system_code = in_system_code;

	IF found THEN
		RETURN;
	END IF;

	BEGIN
		INSERT INTO product_has_os (product_code, system_code) VALUES (in_product_code, in_system_code);
	END;
END;
$$
LANGUAGE plpgsql;

COMMIT;


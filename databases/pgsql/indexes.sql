BEGIN;

CREATE INDEX hash_index_sha1 ON hash (sha1);
CREATE INDEX product_mfg_code_index ON product (mfg_code);
CREATE INDEX fk_os_mfg_code ON os (mfg_code);
CREATE INDEX file_index_product_code ON file (product_code);
CREATE INDEX file_index_op_system_code ON file (op_system_code);
CREATE INDEX file_index_special_code ON file (special_code);
CREATE INDEX file_index_hash_sha1 ON file (hash_sha1);
CREATE INDEX fk_product_has_os_os ON product_has_os (system_code);
CREATE INDEX fk_product_has_os_product ON product_has_os (product_code);

COMMIT;


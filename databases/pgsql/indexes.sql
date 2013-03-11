BEGIN;

-- This is the primary key of the table, PgSQL already creates an implicit index
-- called hash_pkey
-- CREATE INDEX hash_index_sha1 ON hash (sha1);

-- product
CREATE INDEX product_index_mfg_code ON product (mfg_code);
CREATE INDEX product_fk_index_has_os_os ON product_has_os (system_code);
CREATE INDEX product_fk_index_has_os_product ON product_has_os (product_code);

-- os
CREATE INDEX fk_os_index_mfg_code ON os (mfg_code);

-- file
CREATE INDEX file_index_product_code ON file (product_code);
CREATE INDEX file_index_op_system_code ON file (op_system_code);
CREATE INDEX file_index_special_code ON file (special_code);
CREATE INDEX file_index_hash_sha1 ON file (hash_sha1);

COMMIT;


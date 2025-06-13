USE 7life;
DROP TABLE dados;
CREATE TABLE dados(
    dados_id INT PRIMARY KEY AUTO_INCREMENT ,
    use_id INT NOT NULL,
    dados_tipo ENUM("temperatura", "oxigenacao", "bpm") NOT NULL,
    dados_valor VARCHAR(32) NOT NULL,
    dados_generate TIMESTAMP NOT NULL,
    dados_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	FOREIGN KEY (use_id) REFERENCES usuariosEsp(use_id)
);

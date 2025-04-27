-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Creato il: Apr 27, 2025 alle 13:11
-- Versione del server: 10.4.28-MariaDB
-- Versione PHP: 8.2.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `MediaStreaming`
--
CREATE DATABASE IF NOT EXISTS `MediaStreaming` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
USE `MediaStreaming`;

-- --------------------------------------------------------

--
-- Struttura della tabella `episodes`
--

CREATE TABLE `episodes` (
  `episode_id` int(11) NOT NULL,
  `season_id` int(11) DEFAULT NULL,
  `episode_number` int(11) DEFAULT NULL,
  `name` varchar(255) DEFAULT NULL,
  `overview` text DEFAULT NULL,
  `air_date` date DEFAULT NULL,
  `runtime` int(11) DEFAULT NULL,
  `vote_average` float DEFAULT NULL,
  `vote_count` int(11) DEFAULT NULL,
  `still_path` varchar(255) DEFAULT NULL,
  `last_update` date DEFAULT NULL,
  `tv_id` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `genres`
--

CREATE TABLE `genres` (
  `genre_id` int(11) NOT NULL,
  `name` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `movies`
--

CREATE TABLE `movies` (
  `movie_id` int(11) NOT NULL,
  `title` varchar(255) NOT NULL,
  `overview` text DEFAULT NULL,
  `release_date` date DEFAULT NULL,
  `runtime` int(11) DEFAULT NULL,
  `status` varchar(50) DEFAULT NULL,
  `popularity` float DEFAULT NULL,
  `vote_average` float DEFAULT NULL,
  `vote_count` int(11) DEFAULT NULL,
  `poster_path` varchar(255) DEFAULT NULL,
  `backdrop_path` varchar(255) DEFAULT NULL,
  `adult` tinyint(1) DEFAULT NULL,
  `imdb_id` varchar(20) DEFAULT NULL,
  `homepage` varchar(255) DEFAULT NULL,
  `video` tinyint(1) DEFAULT NULL,
  `last_update` date DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `movie_cast`
--

CREATE TABLE `movie_cast` (
  `movie_id` int(11) NOT NULL,
  `person_id` int(11) NOT NULL,
  `character` varchar(255) DEFAULT NULL,
  `credit_id` varchar(255) NOT NULL,
  `order` int(11) DEFAULT NULL,
  `department` varchar(100) DEFAULT NULL,
  `job` varchar(100) DEFAULT NULL,
  `last_update` date DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `movie_genres`
--

CREATE TABLE `movie_genres` (
  `movie_id` int(11) NOT NULL,
  `genre_id` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `people`
--

CREATE TABLE `people` (
  `person_id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL,
  `gender` tinyint(4) DEFAULT NULL,
  `biography` text DEFAULT NULL,
  `birthday` date DEFAULT NULL,
  `deathday` date DEFAULT NULL,
  `known_for_department` varchar(100) DEFAULT NULL,
  `popularity` float DEFAULT NULL,
  `profile_path` varchar(255) DEFAULT NULL,
  `imdb_id` varchar(20) DEFAULT NULL,
  `homepage` varchar(255) DEFAULT NULL,
  `last_update` date DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `seasons`
--

CREATE TABLE `seasons` (
  `season_id` int(11) NOT NULL,
  `tv_id` int(11) DEFAULT NULL,
  `season_number` int(11) DEFAULT NULL,
  `name` varchar(255) DEFAULT NULL,
  `overview` text DEFAULT NULL,
  `air_date` date DEFAULT NULL,
  `poster_path` varchar(255) DEFAULT NULL,
  `episode_count` int(11) DEFAULT NULL,
  `last_update` date DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `telegram_video`
--

CREATE TABLE `telegram_video` (
  `id` int(11) NOT NULL,
  `chat_id` bigint(20) DEFAULT NULL,
  `video_id` bigint(20) DEFAULT NULL,
  `movie_id` int(11) DEFAULT NULL,
  `episode_id` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `tv_shows`
--

CREATE TABLE `tv_shows` (
  `tv_id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL,
  `overview` text DEFAULT NULL,
  `first_air_date` date DEFAULT NULL,
  `last_air_date` date DEFAULT NULL,
  `number_of_seasons` int(11) DEFAULT NULL,
  `number_of_episodes` int(11) DEFAULT NULL,
  `status` varchar(50) DEFAULT NULL,
  `popularity` float DEFAULT NULL,
  `vote_average` float DEFAULT NULL,
  `vote_count` int(11) DEFAULT NULL,
  `poster_path` varchar(255) DEFAULT NULL,
  `backdrop_path` varchar(255) DEFAULT NULL,
  `homepage` varchar(255) DEFAULT NULL,
  `in_production` tinyint(1) DEFAULT NULL,
  `type` varchar(50) DEFAULT NULL,
  `last_update` date DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `tv_show_cast`
--

CREATE TABLE `tv_show_cast` (
  `tv_id` int(11) NOT NULL,
  `person_id` int(11) NOT NULL,
  `character` varchar(255) DEFAULT NULL,
  `credit_id` varchar(255) NOT NULL,
  `order` int(11) DEFAULT NULL,
  `department` varchar(100) DEFAULT NULL,
  `job` varchar(100) DEFAULT NULL,
  `last_update` date DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Struttura della tabella `tv_show_genres`
--

CREATE TABLE `tv_show_genres` (
  `tv_id` int(11) NOT NULL,
  `genre_id` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Indici per le tabelle scaricate
--

--
-- Indici per le tabelle `episodes`
--
ALTER TABLE `episodes`
  ADD PRIMARY KEY (`episode_id`),
  ADD KEY `season_id` (`season_id`),
  ADD KEY `fk_tv_id` (`tv_id`);

--
-- Indici per le tabelle `genres`
--
ALTER TABLE `genres`
  ADD PRIMARY KEY (`genre_id`);

--
-- Indici per le tabelle `movies`
--
ALTER TABLE `movies`
  ADD PRIMARY KEY (`movie_id`);

--
-- Indici per le tabelle `movie_cast`
--
ALTER TABLE `movie_cast`
  ADD PRIMARY KEY (`movie_id`,`person_id`,`credit_id`),
  ADD UNIQUE KEY `credit_id` (`credit_id`),
  ADD KEY `person_id` (`person_id`);

--
-- Indici per le tabelle `movie_genres`
--
ALTER TABLE `movie_genres`
  ADD PRIMARY KEY (`movie_id`,`genre_id`),
  ADD KEY `genre_id` (`genre_id`);

--
-- Indici per le tabelle `people`
--
ALTER TABLE `people`
  ADD PRIMARY KEY (`person_id`);

--
-- Indici per le tabelle `seasons`
--
ALTER TABLE `seasons`
  ADD PRIMARY KEY (`season_id`),
  ADD KEY `tv_id` (`tv_id`);

--
-- Indici per le tabelle `telegram_video`
--
ALTER TABLE `telegram_video`
  ADD PRIMARY KEY (`id`),
  ADD KEY `fk_telegram_video_movie` (`movie_id`),
  ADD KEY `fk_telegram_video_tv_show` (`episode_id`);

--
-- Indici per le tabelle `tv_shows`
--
ALTER TABLE `tv_shows`
  ADD PRIMARY KEY (`tv_id`);

--
-- Indici per le tabelle `tv_show_cast`
--
ALTER TABLE `tv_show_cast`
  ADD PRIMARY KEY (`tv_id`,`person_id`,`credit_id`),
  ADD UNIQUE KEY `credit_id` (`credit_id`),
  ADD KEY `person_id` (`person_id`);

--
-- Indici per le tabelle `tv_show_genres`
--
ALTER TABLE `tv_show_genres`
  ADD PRIMARY KEY (`tv_id`,`genre_id`),
  ADD KEY `genre_id` (`genre_id`);

--
-- AUTO_INCREMENT per le tabelle scaricate
--

--
-- AUTO_INCREMENT per la tabella `telegram_video`
--
ALTER TABLE `telegram_video`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- Limiti per le tabelle scaricate
--

--
-- Limiti per la tabella `episodes`
--
ALTER TABLE `episodes`
  ADD CONSTRAINT `episodes_ibfk_1` FOREIGN KEY (`season_id`) REFERENCES `seasons` (`season_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `fk_tv_id` FOREIGN KEY (`tv_id`) REFERENCES `tv_shows` (`tv_id`);

--
-- Limiti per la tabella `movie_cast`
--
ALTER TABLE `movie_cast`
  ADD CONSTRAINT `movie_cast_ibfk_1` FOREIGN KEY (`movie_id`) REFERENCES `movies` (`movie_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `movie_cast_ibfk_2` FOREIGN KEY (`person_id`) REFERENCES `people` (`person_id`) ON DELETE CASCADE;

--
-- Limiti per la tabella `movie_genres`
--
ALTER TABLE `movie_genres`
  ADD CONSTRAINT `movie_genres_ibfk_1` FOREIGN KEY (`movie_id`) REFERENCES `movies` (`movie_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `movie_genres_ibfk_2` FOREIGN KEY (`genre_id`) REFERENCES `genres` (`genre_id`) ON DELETE CASCADE;

--
-- Limiti per la tabella `seasons`
--
ALTER TABLE `seasons`
  ADD CONSTRAINT `seasons_ibfk_1` FOREIGN KEY (`tv_id`) REFERENCES `tv_shows` (`tv_id`) ON DELETE CASCADE;

--
-- Limiti per la tabella `telegram_video`
--
ALTER TABLE `telegram_video`
  ADD CONSTRAINT `fk_telegram_video_movie` FOREIGN KEY (`movie_id`) REFERENCES `movies` (`movie_id`) ON DELETE SET NULL,
  ADD CONSTRAINT `fk_telegram_video_tv_show` FOREIGN KEY (`episode_id`) REFERENCES `episodes` (`episode_id`) ON DELETE SET NULL;

--
-- Limiti per la tabella `tv_show_cast`
--
ALTER TABLE `tv_show_cast`
  ADD CONSTRAINT `tv_show_cast_ibfk_1` FOREIGN KEY (`tv_id`) REFERENCES `tv_shows` (`tv_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `tv_show_cast_ibfk_2` FOREIGN KEY (`person_id`) REFERENCES `people` (`person_id`) ON DELETE CASCADE;

--
-- Limiti per la tabella `tv_show_genres`
--
ALTER TABLE `tv_show_genres`
  ADD CONSTRAINT `tv_show_genres_ibfk_1` FOREIGN KEY (`tv_id`) REFERENCES `tv_shows` (`tv_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `tv_show_genres_ibfk_2` FOREIGN KEY (`genre_id`) REFERENCES `genres` (`genre_id`) ON DELETE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
# ft_ping

## EN

Post common core project at 42.
A reimplementation of the `ping` command based on **inetutils-2.0** (`ping -V`).

### Features

- ICMP Echo Request / Echo Reply over IPv4
- DNS resolution (hostname and FQDN)
- No reverse DNS on packet return
- Signal handling (Ctrl+C for clean exit and statistics display)

### Options

| Flag | Description |
|------|-------------|
| `-v` | Verbose mode: display ICMP error details |
| `-c count` | Stop after sending `count` packets |
| `-?` | Display usage |

### Build

```
make
```

### Usage

Requires root privileges or `CAP_NET_RAW` capability (raw sockets).

```
sudo ./ft_ping [-v] [-c count] [-?] destination
```

### Examples

```
sudo ./ft_ping -c 3 127.0.0.1
sudo ./ft_ping -v -c 5 google.com
```

---

## FR

Projet post common core a 42.
Reimplementation de la commande `ping` basee sur **inetutils-2.0** (`ping -V`).

### Fonctionnalites

- ICMP Echo Request / Echo Reply en IPv4
- Resolution DNS (hostname et FQDN)
- Pas de resolution DNS inverse au retour du paquet
- Gestion des signaux (Ctrl+C pour arret propre et affichage des statistiques)

### Options

| Flag | Description |
|------|-------------|
| `-v` | Mode verbose : affiche les details des erreurs ICMP |
| `-c count` | Arrete apres l'envoi de `count` paquets |
| `-?` | Affiche l'utilisation |

### Compilation

```
make
```

### Utilisation

Necessite les privileges root ou la capability `CAP_NET_RAW` (raw sockets).

```
sudo ./ft_ping [-v] [-c count] [-?] destination
```

### Exemples

```
sudo ./ft_ping -c 3 127.0.0.1
sudo ./ft_ping -v -c 5 google.com
```

#!/usr/bin/env python3

# This is a python script to sign the plugins

import subprocess
import argparse
from shlex import join, split
import shutil
import zipfile
from pathlib import Path

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


parser = argparse.ArgumentParser(description='Sign the plugins')
parser.add_argument('--sign', action='store_true', help='Sign the plugins')
parser.add_argument('--verify', action='store_true', help='Verify the plugins')
parser.add_argument('--doall', action='store_true', help='Sign, verify, package and notarize the plugins')
parser.add_argument('--plugins', nargs='+', action='store', help='The plugins to sign')
parser.add_argument('--identity', action='store', help='The identity to use, something like "Developer ID Application: Your Name (XXXXXXXXXX)"')
parser.add_argument('--username', action='store', help='The username to use for notarization')
parser.add_argument('--password', action='store', help='The password to use for notarization')
parser.add_argument('--primary-bundle-id', dest="bundleid", action='store', help='The primary bundle id to use for notarization')
parser.add_argument('--team-id', dest="teamid", action='store', help='The team id to use for notarization')
parser.add_argument('--entitlements', action='store', help='The entitlements to use')
parser.add_argument('--requirements', action='store', help='The requirements to use')
parser.add_argument('--verbose', action='store_true', help='Verbose output')
args = parser.parse_args()

def zip_folders(folders, output_path):
    with zipfile.ZipFile(output_path, 'w', compression=zipfile.ZIP_DEFLATED) as zipf:
        for path in folders:
            path = Path(path)
            # get the base folder path for each folder
            base_folder = str(path.parent)
            # iterate through each file in the folder and add it to the zip file
            for file in path.glob('**/*'):
                file_path = str(file)
                # preserve the base folder path when adding files
                zipf.write(file_path, arcname=file_path.replace(base_folder, '', 1))

def sign_plugin(plugin, identity, entitlements, requirements, verbose):

    if verbose:
        print(bcolors.HEADER + "Signing plugin: " + plugin + bcolors.ENDC)

    if entitlements:
        entitlements = f"--entitlements {entitlements}"

    if requirements:
        requirements = f"--requirements {requirements}"

    command = f"codesign --force --deep --timestamp --sign \"{identity}\" {plugin}"
    if verbose:
        print(command)
   
    subprocess.run(split(command), check=True)

def verify_plugin(plugin, verbose):

    if verbose:
        print(bcolors.HEADER + "Verifying plugin: " + plugin + bcolors.ENDC)

    command = f"codesign -dv {plugin}"
    if verbose:
        print(command)
   
    subprocess.run(split(command), check=True)


if args.sign:
    if args.plugins:
        for plugin in args.plugins:
            sign_plugin(plugin, args.identity, args.entitlements, args.requirements, args.verbose)
            verify_plugin(plugin, args.verbose)

if args.verify:
    if args.plugins:
        for plugin in args.plugins:
            verify_plugin(plugin, args.verbose)

if args.doall:
    if args.plugins:
        for plugin in args.plugins:
            sign_plugin(plugin, args.identity, args.entitlements, args.requirements, args.verbose)
            verify_plugin(plugin, args.verbose)

        # Zip all the plugins using shutil
        print(bcolors.HEADER + "Zipping plugins" + bcolors.ENDC)
        zip_folders(args.plugins, "NeuralResonator.zip")

        print(bcolors.HEADER + "Create notarization credentials" + bcolors.ENDC)
        command = f"xcrun notarytool store-credentials \"notarytool-profile\" --apple-id {args.username} --team-id {args.teamid} --password {args.password}"
        if args.verbose:
            print(bcolors.BOLD + command + bcolors.ENDC)
        subprocess.run(split(command), check=True)

        # Notarize the zip file
        print(bcolors.HEADER + "Notarizing plugins" + bcolors.ENDC)
        command = f"xcrun notarytool submit NeuralResonator.zip --keychain-profile \"notarytool-profile\" --wait"
        if args.verbose:
            print(bcolors.BOLD + command + bcolors.ENDC)
        subprocess.run(split(command), check=True)

        # Unzip the notarized zip file in a temp folder
        print(bcolors.HEADER + "Unzipping notarized plugins" + bcolors.ENDC)
        with zipfile.ZipFile("NeuralResonator.zip", 'r') as zip_ref:
            zip_ref.extractall("NeuralResonator")
        
        # Staple the notarization to each plugin
        print(bcolors.HEADER + "Stapling notarization to plugins" + bcolors.ENDC)
        extracted_plugins = Path("NeuralResonator")
        for plugin in extracted_plugins.glob('*/'):
            command = f"xcrun stapler staple {plugin}"
            if args.verbose:
                print(bcolors.BOLD + command + bcolors.ENDC)
            subprocess.run(split(command), check=True)
        
        # Zip the stapled plugins
        print(bcolors.HEADER + "Zipping stapled plugins" + bcolors.ENDC)
        zip_folders(extracted_plugins.glob('*/'), "NeuralResonator_stapled.zip")

        # Delete the temp folder and the notarized zip file
        print(bcolors.HEADER + "Deleting temp folder and notarized zip file" + bcolors.ENDC)
        shutil.rmtree("NeuralResonator")
        Path("NeuralResonator.zip").unlink()




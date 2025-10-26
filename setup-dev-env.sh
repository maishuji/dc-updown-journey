#!/bin/bash
# UDJourney Development Environment Switcher

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

echo "=== UDJourney Development Environment Switcher ==="
echo ""
echo "Choose your development environment:"
echo "1) Linux (Game + Editor development)"
echo "2) Dreamcast (Game development)"
echo "3) Show current configuration"
echo ""

read -p "Enter your choice (1-3): " choice

case $choice in
    1)
        print_info "Setting up Linux development environment..."
        
        # Copy Linux devcontainer config
        if [ -f ".devcontainer/devcontainer-linux.json" ]; then
            cp .devcontainer/devcontainer-linux.json .devcontainer/devcontainer.json
            print_success "Linux devcontainer configuration activated"
        else
            print_warning "Linux devcontainer config not found"
        fi
        
        # Copy Linux VS Code configs
        if [ -f ".vscode/tasks-linux.json" ]; then
            cp .vscode/tasks-linux.json .vscode/tasks.json
            print_success "Linux tasks configuration activated"
        fi
        
        if [ -f ".vscode/launch-linux.json" ]; then
            cp .vscode/launch-linux.json .vscode/launch.json
            print_success "Linux launch configuration activated"
        fi
        
        print_success "Linux environment ready!"
        print_info "To get started:"
        print_info "1. Reopen VS Code in container (Ctrl+Shift+P -> 'Dev Containers: Reopen in Container')"
        print_info "2. Run: ./build-linux.sh"
        print_info "3. Or use VS Code task: Ctrl+Shift+P -> 'Tasks: Run Task' -> 'Build UDJourney Editor (Linux)'"
        ;;
        
    2)
        print_info "Setting up Dreamcast development environment..."
        
        # Restore original Dreamcast devcontainer config
        if [ -f ".devcontainer/devcontainer.json.original" ]; then
            cp .devcontainer/devcontainer.json.original .devcontainer/devcontainer.json
            print_success "Dreamcast devcontainer configuration restored"
        else
            # Create a basic Dreamcast config
            cat > .devcontainer/devcontainer.json << 'EOF'
{
	"name": "Dreamcast Environment",
	"image": "maishuji/dc-kos-image:14.3.0-28sep25-kp29sep25",
	"postCreateCommand": "source /opt/toolchains/dc/kos/environ.sh",
	"customizations": {
		"vscode": {
			"extensions": [
				"ms-vscode.cpptools-extension-pack",
				"donjayamanne.githistory",
				"eamodio.gitlens",
				"github.vscode-github-actions",
				"shd101wyy.markdown-preview-enhanced"
			],
			"settings": {
				"terminal.integrated.shell.linux": "/bin/bash",
				"terminal.integrated.shellArgs.linux": ["--login"]
			}
		}
	},
	"updateRemoteUserUID": true,
	"mounts": [
		"type=bind,source=${localEnv:HOME}/.ssh/,target=/home/non-root/.ssh/,readonly"
	],
	"remoteUser": "non-root"
}
EOF
            print_success "Dreamcast devcontainer configuration activated"
        fi
        
        print_success "Dreamcast environment ready!"
        print_info "Reopen VS Code in container to start Dreamcast development"
        ;;
        
    3)
        print_info "Current configuration:"
        
        if [ -f ".devcontainer/devcontainer.json" ]; then
            if grep -q "UDJourney Linux Development" .devcontainer/devcontainer.json; then
                print_success "Currently set to: Linux Development Environment"
                print_info "Use './build-linux.sh' to build the editor"
            elif grep -q "Dreamcast Environment" .devcontainer/devcontainer.json; then
                print_success "Currently set to: Dreamcast Development Environment"
                print_info "Use KallistiOS build commands for Dreamcast"
            else
                print_warning "Unknown configuration detected"
            fi
        else
            print_warning "No devcontainer configuration found"
        fi
        ;;
        
    *)
        print_warning "Invalid choice. Please run the script again."
        exit 1
        ;;
esac

echo ""
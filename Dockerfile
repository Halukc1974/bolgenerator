# Stage 1: Build C++ backend
FROM ubuntu:22.04 AS builder

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    libocct-data-exchange-dev \
    libocct-draw-dev \
    libocct-foundation-dev \
    libocct-modeling-algorithms-dev \
    libocct-modeling-data-dev \
    libocct-ocaf-dev \
    libocct-visualization-dev \
    occt-misc \
    && rm -rf /var/lib/apt/lists/*

# Build arguments for Coolify/CI
ARG COOLIFY_URL
ARG COOLIFY_FQDN
ARG COOLIFY_BUILD_SECRETS_HASH

WORKDIR /app


# Copy C++ source files and Makefile
COPY Makefile ./
COPY *.cpp *.h ./

# Compile the C++ backend
RUN make clean && make

# Stage 2: Runtime
FROM ubuntu:22.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install Node.js and OCCT runtime libraries
RUN apt-get update && apt-get install -y \
    curl \
    libocct-data-exchange-7.5 \
    libocct-draw-7.5 \
    libocct-foundation-7.5 \
    libocct-modeling-algorithms-7.5 \
    libocct-modeling-data-7.5 \
    libocct-ocaf-7.5 \
    libocct-visualization-7.5 \
    && curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary from builder
COPY --from=builder /app/scim_bolts ./

# Copy Node.js application files
COPY package*.json ./
RUN npm install --production

COPY server.js ./
COPY public ./public

# Create Tests directory for generated files
RUN mkdir -p Tests && chmod 777 Tests

# Expose port
EXPOSE 3000

# Start the application
CMD ["npm", "start"]

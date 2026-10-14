# Pacman relay: docker build -t pacman-relay . && docker run -p 8787:8787 pacman-relay
FROM node:22-alpine
WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci --omit=dev
COPY server.js ./
ENV PORT=8787
EXPOSE 8787
USER node
CMD ["node", "server.js"]
